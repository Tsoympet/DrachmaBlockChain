#include "../../layer1-core/pow/difficulty_adjust.h"
#include "../../layer1-core/consensus/params.h"
#include <cassert>
#include <vector>

int main()
{
    auto params = consensus::Main();
    std::vector<uint32_t> times;
    std::vector<uint32_t> bits;
    uint32_t now = params.nGenesisTime;
    const size_t window = params.nDifficultyAdjustmentInterval;
    for (size_t i = 0; i < window; ++i) {
        now += params.nPowTargetSpacing + (i % 2); // slight jitter
        times.push_back(now);
        bits.push_back(params.nGenesisBits);
    }

    auto retarget = powalgo::CalculateAdaptiveNextWork(times, bits, params);
    assert(retarget.nextBits != 0);
    assert(retarget.averagedTimespan >= static_cast<int64_t>(params.nPowTargetTimespan / 2));
    assert(retarget.anchorBits == params.nGenesisBits);
    return 0;
}
