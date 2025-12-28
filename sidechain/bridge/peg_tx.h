#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sidechain::bridge {

struct lock_tx {
    std::string tx_id;      // unique mainnet transaction identifier
    std::string from;       // mainnet sender locking DRM
    std::string to;         // sidechain recipient to receive wDRM
    uint64_t amount{0};     // amount of DRM locked / wDRM minted
    bool is_lock{false};    // whether the transaction is a recognized lock primitive
};

struct spv_proof {
    std::string tx_id;
    std::vector<std::array<uint8_t, 32>> merkle_path;
    std::array<uint8_t, 32> merkle_root{};
    uint32_t confirmations{0};
    uint32_t mainnet_height{0};
    uint32_t expiration_height{0};
};

struct burn_record {
    std::string burn_id;
    std::string from;
    uint64_t amount{0};
};

struct peg_result {
    bool success{false};
    std::string error;
    uint64_t amount{0};
    burn_record burn;
};

class peg_bridge {
public:
    peg_bridge(uint32_t min_confirmations, uint32_t timeout_blocks);

    peg_result mint_wrapped(const lock_tx& lock,
                            const spv_proof& proof,
                            uint32_t current_sidechain_height);

    peg_result burn_wrapped(const std::string& from, uint64_t amount);

    uint64_t balance_of(const std::string& owner) const;

    bool is_processed(const std::string& tx_id) const;

private:
    bool validate_spv(const lock_tx& lock, const spv_proof& proof) const;
    std::array<uint8_t, 32> double_sha256(const std::vector<uint8_t>& data) const;

    uint32_t min_confirmations_;
    uint32_t timeout_blocks_;
    std::unordered_set<std::string> processed_locks_;
    std::unordered_map<std::string, uint64_t> balances_;
    uint64_t burn_nonce_{0};
};

}  // namespace sidechain::bridge

