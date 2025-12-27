# DRACHMA Roadmap (Draft)

This roadmap outlines milestones for stabilizing the DRACHMA reference implementation. Timelines depend on community review and security results.

## Phase 1 — Foundations
- ✅ Repository layout, licensing, CI skeletons
- 🔄 Complete Layer 1 validation pipeline (blocks, transactions, difficulty)
- 🔄 Deterministic builds and reproducible binaries

## Phase 2 — Networking & Wallets
- Harden P2P message validation and DoS protections
- Implement mempool policy and fee estimation
- Deliver wallet backend with address/key management and transaction construction

## Phase 3 — Mining & Tooling
- Optimize reference CPU/GPU miners with reproducible build scripts
- Add monitoring/metrics endpoints for nodes and miners
- Provide deployment scripts (systemd, containers)

## Phase 4 — Testnet
- Launch public testnet with seed nodes and explorers
- Enable crash reporting and telemetry opt-in (if any) with privacy safeguards
- Capture performance metrics and adjust parameters

## Phase 5 — Security Review
- Commission third-party audits of consensus, networking, and wallet code
- Run adversarial test suites and fuzzing campaigns
- Publish threat models and remediation plans

## Phase 6 — Release Candidates
- Produce signed binaries for major platforms
- Document upgrade procedures and rollback plans
- Finalize activation parameters for any consensus changes

## Phase 7 — Mainnet
- Execute fair mainnet launch with open-source tooling
- Monitor chain stability; apply hotfixes if critical
- Plan long-term maintenance and governance-free sustainability

Progress updates and deviations from this plan will be tracked in issues and release notes.
