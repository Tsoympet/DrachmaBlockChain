#pragma once
#include <cstdint>
#include <vector>
#include "difficulty.h"
#include "../consensus/params.h"

namespace powalgo {

struct RetargetMetrics {
    uint32_t nextBits;
    int64_t averagedTimespan;
    uint32_t anchorBits;
};

// Compute an adaptive difficulty target using a median-of-three window to
// smooth timestamp variance and a bounded dampening factor to resist 51%
// oscillation attacks. The caller must supply the timestamps (newest last)
// and associated compact targets for the last N blocks (at least 3 entries).
RetargetMetrics CalculateAdaptiveNextWork(
    const std::vector<uint32_t>& timestamps,
    const std::vector<uint32_t>& bits,
    const consensus::Params& params);

// Helper that clamps a timespan to the allowable window for the network.
int64_t ClampRetargetTimespan(int64_t span, const consensus::Params& params);

}
