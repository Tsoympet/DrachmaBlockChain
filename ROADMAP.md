# DRACHMA Project Roadmap

A phased plan for delivering the DRACHMA blockchain with transparency and predictable milestones.

## Status Overview

- [x] Implement Layer 1 consensus, UTXO, and PoW with difficulty retargeting
- [x] Harden fork resolution (checkpoints optional), timestamp bounds, and validation rate limits
- [x] Stand up public **testnet** with seeds, faucet, explorer, and Qt wallet builds
- [x] Layer 2 services: P2P, mempool, RPC surface, and wallet backend
- [x] Reference miners with Stratum support and benchmarking
- [ ] External audits (consensus, networking, wallet, miners)
- [ ] Mainnet parameter freeze and launch ceremony
- [ ] Long-term monitoring/observability package for operators

## 2024 Q4 — Testnet Maturation
- Expand fuzzing/integration coverage and publish updated threat models.
- Ship reproducible release builds (Docker + native) for testnet nodes and miners.
- Iterate on wallet UX (encryption, backups, address book, QR flows) with community feedback.

## 2025 Q1 — Audit & Mainnet Readiness
- Commission third-party audits covering consensus, networking, and wallet components.
- Finalize genesis parameters and publish deterministic build scripts for launch artifacts.
- Freeze mainnet rule set; release RC binaries and documentation for operators.

## 2025 Q2 — Mainnet Launch
- Execute a fair mainnet launch with transparent participation criteria and public seed lists.
- Publish monitoring dashboards and on-call playbooks for early operators.
- Deliver public block explorer and supply dashboards synchronized with launch.

## Post-Launch (Ongoing)
- Maintain a regular documentation cadence and responsible disclosure workflow.
- Track operational metrics (orphan rate, mempool pressure, fee market health) and propose non-consensus mitigations as needed.
- Encourage ecosystem clients/wallets to reuse the reference implementation’s validation logic to avoid consensus drift.
