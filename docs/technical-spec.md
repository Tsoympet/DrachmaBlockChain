# DRACHMA Technical Specification (Draft)

This document summarizes key protocol parameters for DRACHMA. Values may be finalized during testnet iterations.

## Monetary Policy

- **Supply Cap:** 42,000,000 DRM
- **Block Subsidy (initial):** 50 DRM (testnet placeholder)
- **Halving Interval:** 210,000 blocks (approx. every 4 years at 10-minute blocks)
- **Minimum Subsidy:** 0 DRM (tail emission not planned)
- **Fee Model:** Transaction fees are collected by miners in full; no fee burning.

## Consensus Parameters

- **Proof of Work:** SHA-256d
- **Target Block Time:** 10 minutes
- **Difficulty Adjustment:** Retarget every 2016 blocks (approx. 2 weeks), bounded to limit abrupt swings
- **Block Size/Weight:** To be finalized with performance measurements; conservative defaults favored
- **Timestamp Rules:** Median-time-past enforcement and future drift limits consistent with Bitcoin-like safety

## Transaction Model

- **UTXO-based ledger** with deterministic script-less validation
- **Signatures:** Schnorr over secp256k1
- **Hashing:** Tagged SHA-256 for transaction IDs; SHA-256d for Merkle roots
- **Standardness Policies:** Defined at Layer 2 (mempool) and must not modify consensus validity rules

## Network Parameters

- **Default Ports:**
  - P2P: 9333 (testnet example)
  - RPC: 8332
  - Work/Stratum-like mining endpoint: 9333 (when proxied via services)
- **Message Integrity:** Checksummed payloads; DoS-resistant bounds on inventories and mempool admission
- **Address Format:** To be defined (e.g., Bech32m variant) with checksum for user safety

## Storage

- **Block Files:** Append-only with periodic compaction tools
- **Chainstate:** Key-value store (LevelDB/SQLite variant) with snapshotting support under evaluation
- **Indexes:** Optional transaction/address indexes provided by Layer 2 services only

## Upgrade Policy

- Consensus modifications require:
  - Public proposal with rationale and testing evidence
  - Testnet activation with monitoring
  - Clear activation mechanism (flag day or version bits) and rollback plan

## Non-Goals

- No on-chain smart contracts or programmable VM
- No privileged keys or governance layers
- No premines or developer fees embedded in consensus

Refer to [`docs/whitepaper.md`](whitepaper.md) and [`docs/architecture.md`](architecture.md) for additional context and motivations.
