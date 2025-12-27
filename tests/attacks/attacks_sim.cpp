#include "../../layer1-core/consensus/fork_resolution.h"
#include "../../layer1-core/consensus/params.h"
#include "../../layer1-core/block/block.h"
#include "../../layer1-core/pow/difficulty.h"
#include <cassert>
#include <vector>

namespace {
BlockHeader Make(uint256 parent, uint32_t t, uint32_t bits)
{
    BlockHeader h{};
    h.version = 1;
    h.prevBlockHash = parent;
    h.merkleRoot.fill(0);
    h.time = t;
    h.bits = bits;
    h.nonce = 0;
    return h;
}
}

// Basic harness that mimics a 51% attacker attempting to outpace honest miners
// with higher work blocks and forces a reorg once sufficient cumulative work is
// built. The comments explain where the fork would be rejected if validation
// failed, providing a quick smoke test for manual simulations.
int main()
{
    auto params = consensus::Main();
    consensus::ForkResolver resolver(/*finalizationDepth=*/5, /*reorgWorkMarginBps=*/750);

    uint256 nullHash{};
    auto genesis = Make(nullHash, params.nGenesisTime, params.nGenesisBits);
    auto gHash = BlockHash(genesis);
    assert(resolver.ConsiderHeader(genesis, gHash, nullHash, 0, params));

    // Honest chain produces a handful of blocks at baseline difficulty.
    std::vector<uint256> honestHashes{gHash};
    auto header = genesis;
    for (uint32_t h = 1; h <= 6; ++h) {
        header = Make(honestHashes.back(), header.time + params.nPowTargetSpacing, params.nGenesisBits);
        honestHashes.push_back(BlockHash(header));
        assert(resolver.ConsiderHeader(header, honestHashes.back(), honestHashes[h-1], h, params));
    }

    // Attacker with majority hash power mines a denser fork starting from
    // height 3 using tougher bits to accumulate more work per block.
    uint32_t attackerBits = params.nGenesisBits - 0x020000;
    uint256 forkBase = honestHashes[3];
    uint32_t attackerTime = header.time - params.nPowTargetSpacing; // starts earlier

    std::vector<uint256> attackerHashes;
    for (uint32_t h = 4; h <= 9; ++h) {
        auto blk = Make(attackerHashes.empty() ? forkBase : attackerHashes.back(), ++attackerTime, attackerBits);
        auto hash = BlockHash(blk);
        bool becameTip = resolver.ConsiderHeader(blk, hash, attackerHashes.empty() ? forkBase : attackerHashes.back(), h, params);
        attackerHashes.push_back(hash);
        if (h == 9)
            assert(becameTip); // attacker wins with deeper, higher-work fork
    }

    auto tip = resolver.Tip();
    assert(tip);
    auto path = resolver.ReorgPath(tip->hash);
    assert(path.front() == gHash);
    return 0;
}
