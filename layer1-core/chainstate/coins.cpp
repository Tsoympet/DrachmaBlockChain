#include "coins.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cstring>

#ifdef DRACHMA_HAVE_LEVELDB
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#endif

std::size_t OutPointHash::operator()(const OutPoint& o) const noexcept
{
    size_t h = 0;
    for (auto b : o.hash) h = (h * 131) ^ b;
    h ^= static_cast<size_t>(o.index + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2));
    return h;
}

bool OutPointEq::operator()(const OutPoint& a, const OutPoint& b) const noexcept
{
    return a.index == b.index && std::equal(a.hash.begin(), a.hash.end(), b.hash.begin());
}

Chainstate::Chainstate(const std::string& path, std::size_t cacheCapacity)
    : storagePath(path), maxCacheEntries(cacheCapacity)
{
#ifdef DRACHMA_HAVE_LEVELDB
    leveldb::Options opts;
    opts.create_if_missing = true;
    leveldb::DB* rawDb = nullptr;
    auto status = leveldb::DB::Open(opts, storagePath + ".ldb", &rawDb);
    if (status.ok()) {
        db.reset(rawDb);
        useDb = true;
    }
#endif
    Load();
}

bool Chainstate::HaveUTXO(const OutPoint& out) const
{
    std::lock_guard<std::mutex> l(mu);
    if (cache.find(out) != cache.end())
        return true;
    return utxos.find(out) != utxos.end();
}

std::optional<TxOut> Chainstate::TryGetUTXO(const OutPoint& out) const
{
    std::lock_guard<std::mutex> l(mu);
    auto itCache = cache.find(out);
    if (itCache != cache.end())
        return itCache->second;

    auto it = utxos.find(out);
    if (it == utxos.end())
        return std::nullopt;

    cache.emplace(out, it->second);
    MaybeEvict();
    return it->second;
}

TxOut Chainstate::GetUTXO(const OutPoint& out) const
{
    auto coin = TryGetUTXO(out);
    if (!coin)
        throw std::runtime_error("missing utxo");
    return *coin;
}

void Chainstate::AddUTXO(const OutPoint& out, const TxOut& txout)
{
    std::lock_guard<std::mutex> l(mu);
    utxos[out] = txout;
    cache[out] = txout;
#ifdef DRACHMA_HAVE_LEVELDB
    if (useDb) {
        leveldb::WriteBatch batch;
        // value layout: [value(8)][scriptPubKey]
        std::string value;
        value.resize(sizeof(txout.value));
        std::memcpy(value.data(), &txout.value, sizeof(txout.value));
        value.append(reinterpret_cast<const char*>(txout.scriptPubKey.data()), txout.scriptPubKey.size());

        std::string key;
        key.reserve(out.hash.size() + sizeof(out.index));
        key.append(reinterpret_cast<const char*>(out.hash.data()), out.hash.size());
        key.append(reinterpret_cast<const char*>(&out.index), sizeof(out.index));
        batch.Put(key, value);
        PersistBatch(batch);
    }
#endif
    MaybeEvict();
}

void Chainstate::SpendUTXO(const OutPoint& out)
{
    std::lock_guard<std::mutex> l(mu);
    auto it = utxos.find(out);
    if (it == utxos.end()) throw std::runtime_error("spend missing utxo");
    utxos.erase(it);
    cache.erase(out);
#ifdef DRACHMA_HAVE_LEVELDB
    if (useDb) {
        leveldb::WriteBatch batch;
        std::string key;
        key.reserve(out.hash.size() + sizeof(out.index));
        key.append(reinterpret_cast<const char*>(out.hash.data()), out.hash.size());
        key.append(reinterpret_cast<const char*>(&out.index), sizeof(out.index));
        batch.Delete(key);
        PersistBatch(batch);
    }
#endif
}

void Chainstate::Flush() const { Persist(); }

std::size_t Chainstate::CachedEntries() const
{
    std::lock_guard<std::mutex> l(mu);
    return cache.size();
}

void Chainstate::Load()
{
    std::lock_guard<std::mutex> l(mu);
#ifdef DRACHMA_HAVE_LEVELDB
    if (useDb) {
        std::unique_ptr<leveldb::Iterator> it(db->NewIterator(leveldb::ReadOptions()));
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            const auto& key = it->key();
            const auto& val = it->value();
            OutPoint op{};
            if (key.size() != op.hash.size() + sizeof(uint32_t) || val.size() < sizeof(uint64_t))
                continue;
            std::memcpy(op.hash.data(), key.data(), op.hash.size());
            std::memcpy(&op.index, key.data() + op.hash.size(), sizeof(op.index));
            TxOut txo{};
            std::memcpy(&txo.value, val.data(), sizeof(txo.value));
            txo.scriptPubKey.assign(val.data() + sizeof(txo.value), val.data() + val.size());
            utxos.emplace(op, txo);
        }
        return;
    }
#endif
    std::ifstream in(storagePath, std::ios::binary);
    if (!in.good()) return;
    uint32_t count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    for (uint32_t i = 0; i < count; ++i) {
        OutPoint op{};
        in.read(reinterpret_cast<char*>(op.hash.data()), op.hash.size());
        in.read(reinterpret_cast<char*>(&op.index), sizeof(op.index));
        TxOut txo{};
        in.read(reinterpret_cast<char*>(&txo.value), sizeof(txo.value));
        uint32_t scriptSize = 0;
        in.read(reinterpret_cast<char*>(&scriptSize), sizeof(scriptSize));
        txo.scriptPubKey.resize(scriptSize);
        in.read(reinterpret_cast<char*>(txo.scriptPubKey.data()), scriptSize);
        if (!in) throw std::runtime_error("corrupt utxo set");
        utxos.emplace(op, txo);
    }
}

void Chainstate::Persist() const
{
    std::lock_guard<std::mutex> l(mu);
#ifdef DRACHMA_HAVE_LEVELDB
    if (useDb) {
        return; // LevelDB writes are handled incrementally in Add/Spend
    }
#endif
    std::ofstream out(storagePath, std::ios::binary | std::ios::trunc);
    uint32_t count = static_cast<uint32_t>(utxos.size());
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& entry : utxos) {
        out.write(reinterpret_cast<const char*>(entry.first.hash.data()), entry.first.hash.size());
        out.write(reinterpret_cast<const char*>(&entry.first.index), sizeof(entry.first.index));
        out.write(reinterpret_cast<const char*>(&entry.second.value), sizeof(entry.second.value));
        uint32_t scriptSize = static_cast<uint32_t>(entry.second.scriptPubKey.size());
        out.write(reinterpret_cast<const char*>(&scriptSize), sizeof(scriptSize));
        out.write(reinterpret_cast<const char*>(entry.second.scriptPubKey.data()), scriptSize);
    }
}

void Chainstate::MaybeEvict() const
{
    if (cache.size() <= maxCacheEntries)
        return;

    // Evict the oldest entries deterministically to prevent unbounded growth
    // and keep hot coins resident.
    const size_t target = maxCacheEntries / 2;
    auto it = cache.begin();
    while (cache.size() > target && it != cache.end()) {
        it = cache.erase(it);
    }
}

#ifdef DRACHMA_HAVE_LEVELDB
void Chainstate::PersistBatch(leveldb::WriteBatch& batch) const
{
    if (!useDb)
        return;
    leveldb::WriteOptions opts;
    opts.sync = true; // durability for mainnet safety
    auto status = db->Write(opts, &batch);
    if (!status.ok())
        throw std::runtime_error("leveldb write failed: " + status.ToString());
}
#endif
