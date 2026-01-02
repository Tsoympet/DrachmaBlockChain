#include "../../layer1-core/consensus/params.h"
#include <cassert>

namespace {
constexpr uint64_t COIN = 100000000ULL;
}

int main()
{
    const auto& params = consensus::Testnet();

    // TLN is PoW-only; PoS reward must be zero.
    auto tlnReward = consensus::GetPoSReward(100 * COIN, params, static_cast<uint8_t>(AssetId::TALANTON));
    assert(tlnReward == 0);

    // DRM PoS: 4% APR with 10-minute slots.
    const double slotsPerYear = 31536000.0 / 600.0;
    uint64_t stake = 100 * COIN;
    uint64_t expectedDrm = static_cast<uint64_t>((static_cast<double>(stake) * 0.04) / slotsPerYear);
    auto drmReward = consensus::GetPoSReward(stake, params, static_cast<uint8_t>(AssetId::DRACHMA));
    assert(drmReward == expectedDrm);

    // OBL uses Eth2-style curve; for small participation it should exceed DRM's per-slot reward.
    auto oblReward = consensus::GetPoSReward(stake, params, static_cast<uint8_t>(AssetId::OBOLOS));
    assert(oblReward > drmReward);
    assert(oblReward > 0);

    return 0;
}
