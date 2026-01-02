#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/multiprecision/cpp_int.hpp>

struct evm_word_hash {
    std::size_t operator()(const boost::multiprecision::cpp_int& value) const noexcept;
};

struct evm_code {
    std::vector<uint8_t> bytes;
};

struct evm_state {
    std::unordered_map<boost::multiprecision::cpp_int, boost::multiprecision::cpp_int, evm_word_hash> storage;
    uint64_t drm_fee_per_gas{0};
    std::array<uint8_t, 33> validator_pubkey{};
};

struct evm_result {
    bool success{false};
    uint64_t gas_used{0};
    std::vector<uint8_t> return_data;
    std::unordered_map<boost::multiprecision::cpp_int, boost::multiprecision::cpp_int, evm_word_hash> storage;
    std::string error;
};

// EVM execution is disabled; the WASM sidechain is mandatory.
evm_result execute(const evm_code& bytecode, const evm_state& state, uint64_t gas_limit);
