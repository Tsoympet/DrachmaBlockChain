#include "sidechain/bridge/peg_tx.h"

#include <openssl/sha.h>


namespace sidechain::bridge {
namespace {
std::vector<uint8_t> hex_to_bytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        const auto byte = static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

}  // namespace

peg_bridge::peg_bridge(uint32_t min_confirmations, uint32_t timeout_blocks)
    : min_confirmations_(min_confirmations), timeout_blocks_(timeout_blocks) {}

std::array<uint8_t, 32> peg_bridge::double_sha256(const std::vector<uint8_t>& data) const {
    std::array<uint8_t, 32> first{};
    SHA256(data.data(), data.size(), first.data());

    std::array<uint8_t, 32> second{};
    SHA256(first.data(), first.size(), second.data());
    return second;
}

bool peg_bridge::validate_spv(const lock_tx& lock, const spv_proof& proof) const {
    if (!lock.is_lock) {
        return false;
    }
    if (proof.tx_id != lock.tx_id) {
        return false;
    }
    if (proof.confirmations < min_confirmations_) {
        return false;
    }

    // Recompute tx hash from the lock identifier to validate the merkle path.
    std::vector<uint8_t> tx_bytes;
    const auto lock_bytes = hex_to_bytes(lock.tx_id);
    if (!lock_bytes.empty()) {
        tx_bytes = lock_bytes;
    } else {
        tx_bytes.assign(lock.tx_id.begin(), lock.tx_id.end());
    }

    auto hash = double_sha256(tx_bytes);
    for (const auto& sibling : proof.merkle_path) {
        std::vector<uint8_t> concat(hash.begin(), hash.end());
        concat.insert(concat.end(), sibling.begin(), sibling.end());
        hash = double_sha256(concat);
    }

    return hash == proof.merkle_root;
}

peg_result peg_bridge::mint_wrapped(const lock_tx& lock,
                                    const spv_proof& proof,
                                    uint32_t current_sidechain_height) {
    peg_result result{};

    if (processed_locks_.count(lock.tx_id) != 0U) {
        result.error = "replay detected";
        return result;
    }

    if (current_sidechain_height > proof.expiration_height ||
        current_sidechain_height > proof.mainnet_height + timeout_blocks_) {
        result.error = "proof expired";
        return result;
    }

    if (!validate_spv(lock, proof)) {
        result.error = "invalid SPV proof";
        return result;
    }

    processed_locks_.insert(lock.tx_id);
    balances_[lock.to] += lock.amount;

    result.success = true;
    result.amount = lock.amount;
    return result;
}

peg_result peg_bridge::burn_wrapped(const std::string& from, uint64_t amount) {
    peg_result result{};
    auto it = balances_.find(from);
    if (it == balances_.end() || it->second < amount) {
        result.error = "insufficient balance";
        return result;
    }

    it->second -= amount;
    if (it->second == 0) {
        balances_.erase(it);
    }

    burn_record record{};
    record.from = from;
    record.amount = amount;
    record.burn_id = "burn-" + std::to_string(++burn_nonce_);

    result.success = true;
    result.amount = amount;
    result.burn = record;
    return result;
}

uint64_t peg_bridge::balance_of(const std::string& owner) const {
    auto it = balances_.find(owner);
    if (it == balances_.end()) {
        return 0;
    }
    return it->second;
}

bool peg_bridge::is_processed(const std::string& tx_id) const {
    return processed_locks_.count(tx_id) != 0U;
}

}  // namespace sidechain::bridge

