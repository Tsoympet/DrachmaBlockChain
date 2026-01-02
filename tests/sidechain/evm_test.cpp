#include "../../sidechain/evm/evm.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

TEST(EvmExecute, DisabledInFavorOfWasm) {
    evm_code code{{0x00}};
    evm_state state{};

    const auto result = execute(code, state, 1000);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());
    EXPECT_NE(result.error.find("WASM"), std::string::npos);
}
