#include <gtest/gtest.h>
#include "../../layer1-core/pow/difficulty.h"
#include "../../layer1-core/consensus/params.h"

TEST(Difficulty, ClampsExtremeTimespans)
{
    const auto& params = consensus::Main();
    // Use lastBits from genesis for repeatability
    uint32_t base = params.nGenesisBits;
    int64_t veryFast = params.nPowTargetTimespan / 10; // should clamp to /2
    int64_t verySlow = params.nPowTargetTimespan * 10; // should clamp to *2

    uint32_t fast = powalgo::CalculateNextWorkRequired(base, veryFast, params);
    uint32_t slow = powalgo::CalculateNextWorkRequired(base, verySlow, params);

    EXPECT_NE(fast, 0u);
    EXPECT_NE(slow, 0u);
    // New work should still remain within the pow limit and move in expected direction
    EXPECT_LE(fast, base);
    EXPECT_GE(slow, base);
}

TEST(Difficulty, RejectsOverflowTargets)
{
    const auto& params = consensus::Main();
    uint256 hash{};
    hash.fill(0xFF);
    // Pow limit from params.nGenesisBits should reject higher target
    EXPECT_FALSE(powalgo::CheckProofOfWork(hash, params.nGenesisBits - 1, params));
}

TEST(Difficulty, AppliesMinDifficultyForStaleBlocks)
{
    auto params = consensus::Testnet();
    powalgo::BlockIndex prev{};
    powalgo::BlockIndex older{};
    older.height = 0;
    older.time = 100;
    older.bits = params.nGenesisBits;
    prev.prev = &older;
    prev.height = 1;
    prev.time = older.time + params.nPowTargetSpacing * 3; // stale gap triggers min difficulty
    prev.bits = params.nGenesisBits - 0x010000;
    auto next = powalgo::calculate_next_work_required(params, &prev);
    EXPECT_EQ(next, params.nGenesisBits);
}

TEST(Difficulty, ThrowsWhenTargetTimespanZero)
{
    auto params = consensus::Main();
    params.nPowTargetTimespan = 0;
    EXPECT_THROW(powalgo::CalculateNextWorkRequired(params.nGenesisBits, 1, params), std::runtime_error);
}

TEST(Difficulty, RetargetsClampToPowLimitAndAreDeterministic)
{
    const auto& params = consensus::Main();
    // Extremely slow span should clamp to 2x and never exceed pow limit.
    uint32_t eased = powalgo::CalculateNextWorkRequired(params.nGenesisBits, params.nPowTargetTimespan * 100, params);
    EXPECT_GE(eased, params.nGenesisBits);
    uint32_t again = powalgo::CalculateNextWorkRequired(params.nGenesisBits, params.nPowTargetTimespan * 100, params);
    EXPECT_EQ(eased, again);
}

TEST(Difficulty, NegativeTimespansClampToMinimumInterval)
{
    auto params = consensus::Main();
    powalgo::BlockIndex first{};
    powalgo::BlockIndex last{};
    first.height = 0;
    first.time = 1000;
    first.bits = params.nGenesisBits;
    last.prev = &first;
    last.height = static_cast<int>(params.nDifficultyAdjustmentInterval) - 1;
    last.time = 900; // time moved backwards
    last.bits = params.nGenesisBits;
    auto next = powalgo::calculate_next_work_required(params, &last);
    EXPECT_LT(next, params.nGenesisBits); // harder because span clamps to minimum
}
