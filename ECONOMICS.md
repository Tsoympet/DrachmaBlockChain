# DRACHMA Token Economics

A concise overview of supply, issuance, and fair-launch mechanics for the DRACHMA blockchain.

## Supply Overview
- **Ticker:** DRACHMA (DRA)
- **Total Supply Cap:** 42,000,000 DRA
- **Circulating Supply at Launch:** 0 DRA (all issuance via on-chain mining)

## Issuance Model
- **Consensus Layer:** Layer 1 proof-of-work minting through block rewards.
- **Starting Block Reward:** 20 DRA per block.
- **Halving Interval:** Every 1,050,000 blocks (~1 year at ~30s block time).
- **Emission Curve:** Geometric halving until block rewards asymptotically approach zero, converging to the 42M cap.

| Era | Block Range (inclusive) | Reward per Block (DRA) | Era Emission (DRA) | Cumulative Supply (DRA) |
| --- | ----------------------- | ---------------------- | ------------------- | ----------------------- |
| 0 | 0 – 1,049,999 | 20.0 | 21,000,000 | 21,000,000 |
| 1 | 1,050,000 – 2,099,999 | 10.0 | 10,500,000 | 31,500,000 |
| 2 | 2,100,000 – 3,149,999 | 5.0 | 5,250,000 | 36,750,000 |
| 3 | 3,150,000 – 4,199,999 | 2.5 | 2,625,000 | 39,375,000 |
| 4 | 4,200,000 – 5,249,999 | 1.25 | 1,312,500 | 40,687,500 |
| 5+ | 5,250,000 onward | 0.625 → 0 over time | ~1,312,500 (long tail) | 42,000,000 cap |

## Fair Launch Commitments
- **Genesis Block:** Contains only protocol setup transactions; no premine or founder allocations.
- **Miner Access:** Open-source clients and miner reference implementations released ahead of launch to ensure open participation.
- **Faucet on Testnet:** Test assets available solely for development; no testnet balances migrate to mainnet.
- **Transparency:** Public release of genesis block configuration, block reward schedule, and audit reports prior to mainnet activation.

## Distribution Integrity
- Block rewards are the sole issuance mechanism; there are no stealth mints, developer rewards, or advisor grants embedded in the protocol.
- Difficulty adjustment and validation rules are enforced at launch to prevent timestamp or reward manipulation.
- Wallets and explorers will clearly display circulating supply and remaining issuance to maintain user trust.
