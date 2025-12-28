#include "nft.h"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace sidechain::precompiles {
namespace {
constexpr uint64_t k_gas_mint = 50'000;
constexpr uint64_t k_gas_transfer = 25'000;
constexpr uint64_t k_gas_owner_of = 5'000;

std::array<uint8_t, 32> to_bytes32(const uint256& value) {
    std::array<uint8_t, 32> out{};
    uint256 tmp = value;
    for (int i = 31; i >= 0; --i) {
        out[static_cast<size_t>(i)] = static_cast<uint8_t>((tmp & 0xff).convert_to<uint64_t>());
        tmp >>= 8;
    }
    return out;
}

uint256 mask_word(const uint256& value) {
    return value & ((uint256(1) << 256) - 1);
}

bool addresses_equal(const address& a, const address& b) {
    return std::equal(a.begin(), a.end(), b.begin());
}

}  // namespace

nft_precompile::nft_precompile(const std::string& db_path) {
    leveldb::Options opts;
    opts.create_if_missing = true;
    leveldb::DB* raw_db = nullptr;
    auto status = leveldb::DB::Open(opts, db_path, &raw_db);
    if (!status.ok()) {
        throw std::runtime_error("failed to open NFT precompile DB: " + status.ToString());
    }
    db_.reset(raw_db);
}

std::string nft_precompile::encode_token_key(const uint256& token_id) const {
    const auto bytes = to_bytes32(mask_word(token_id));
    std::string key = "nft:owner:";
    key.append(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    return key;
}

std::string nft_precompile::encode_address(const address& addr) {
    return std::string(reinterpret_cast<const char*>(addr.data()), addr.size());
}

bool nft_precompile::decode_address(const std::string& value, address& out) {
    if (value.size() != out.size()) {
        return false;
    }
    std::copy(value.begin(), value.end(), out.begin());
    return true;
}

nft_result nft_precompile::mint(const uint256& token_id, const address& to) {
    nft_result result{};
    result.gas_used = k_gas_mint;

    const std::string token_key = encode_token_key(token_id);
    std::string existing;
    auto status = db_->Get(leveldb::ReadOptions(), token_key, &existing);
    if (status.ok()) {
        result.error = "token already minted";
        return result;
    }

    status = db_->Put(leveldb::WriteOptions(), token_key, encode_address(to));
    if (!status.ok()) {
        result.error = status.ToString();
        return result;
    }

    result.success = true;
    result.owner = to;
    return result;
}

nft_result nft_precompile::transfer(const address& from, const address& to, const uint256& token_id) {
    nft_result result{};
    result.gas_used = k_gas_transfer;

    const std::string token_key = encode_token_key(token_id);
    std::string stored_owner;
    auto status = db_->Get(leveldb::ReadOptions(), token_key, &stored_owner);
    if (!status.ok()) {
        result.error = "token not minted";
        return result;
    }

    address current_owner{};
    if (!decode_address(stored_owner, current_owner)) {
        result.error = "corrupt owner entry";
        return result;
    }

    if (!addresses_equal(current_owner, from)) {
        result.error = "transfer not authorized";
        return result;
    }

    status = db_->Put(leveldb::WriteOptions(), token_key, encode_address(to));
    if (!status.ok()) {
        result.error = status.ToString();
        return result;
    }

    result.success = true;
    result.owner = to;
    return result;
}

nft_result nft_precompile::owner_of(const uint256& token_id) const {
    nft_result result{};
    result.gas_used = k_gas_owner_of;

    const std::string token_key = encode_token_key(token_id);
    std::string stored_owner;
    auto status = db_->Get(leveldb::ReadOptions(), token_key, &stored_owner);
    if (!status.ok()) {
        result.error = "token not minted";
        return result;
    }

    address owner_address{};
    if (!decode_address(stored_owner, owner_address)) {
        result.error = "corrupt owner entry";
        return result;
    }

    result.success = true;
    result.owner = owner_address;
    return result;
}

/*
Example Solidity-like wrapper (for illustration):

pragma solidity ^0.8.0;
contract NativeNFT {
    function mint(uint256 tokenId, address to) external {
        // Calls the NFT precompile address 0x00000000000000000000000000000000000000F1
    }
    function transfer(address from, address to, uint256 tokenId) external {
        // Delegates to the precompile transfer
    }
    function ownerOf(uint256 tokenId) external view returns (address) {
        // Reads via the precompile
    }
}

Example JSON-RPC invocation against the precompile (eth_call style):
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "eth_call",
  "params": [{
    "to": "0x00000000000000000000000000000000000000F1",
    "data": "0x6352211e" // ownerOf selector + encoded token id
  }, "latest"]
}
*/

}  // namespace sidechain::precompiles

