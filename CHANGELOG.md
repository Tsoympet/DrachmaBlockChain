# Changelog

All notable changes to this project will be documented in this file. Dates use ISO 8601 format.

## v0.1.0-alpha - 2024-05-01
- First public alpha with layer1 consensus engine, Schnorr signing, PoW, and initial wallet/GUI.
- Introduced miner reference implementations (CPU/GPU) and basic RPC surface.

## v0.1.0-testnet - 2024-11-15
- Public testnet launch with published genesis, checkpoints, and DNS/bootstrap seeds.
- Added multi-node docker-compose harness, faucet, monitoring, and deployment guides.
- Hardened consensus with checkpoint-aware fork resolution, median-time-past, and DoS guards.

## v0.1.0-rc1 - 2024-12-15
- Release automation with signed binaries for Linux/macOS/Windows.
- Simple web block explorer consuming JSON-RPC, plus operational scripts for upgrades and sync checks.
- Expanded test coverage (P2P, RPC, mempool stress) and economics/technical documentation updates.
