#include "sidechain/evm/evm.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace {
uint64_t load_u256_be(const std::vector<uint8_t>& data) {
    uint64_t value = 0;
    const size_t len = std::min<size_t>(data.size(), 32);
    for (size_t i = 0; i < len; ++i) {
        value = (value << 8) | data[i];
    }
    return value;
}
}

TEST(EvmExecute, AddsTwoNumbersAndReturns) {
    evm_code code{{0x60, 0x02,       // PUSH1 0x02
                   0x60, 0x03,       // PUSH1 0x03
                   0x01,             // ADD
                   0x60, 0x00,       // PUSH1 0x00 (offset)
                   0x52,             // MSTORE
                   0x60, 0x20,       // PUSH1 0x20 (length)
                   0x60, 0x00,       // PUSH1 0x00 (offset)
                   0xf3}};           // RETURN

    evm_state state{};
    state.drm_fee_per_gas = 2;

    const auto result = execute(code, state, 50000);
    ASSERT_TRUE(result.success) << result.error;
    EXPECT_LT(result.gas_used, 50000u);
    ASSERT_EQ(result.return_data.size(), 32u);
    const uint64_t sum = load_u256_be(result.return_data);
    EXPECT_EQ(sum, 5u);
}

TEST(EvmExecute, RejectsUnsupportedOpcode) {
    evm_code code{{0xf1}};  // CALL is disabled
    evm_state state{};

    const auto result = execute(code, state, 1000);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());
}

