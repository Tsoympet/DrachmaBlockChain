# Changelog

All notable changes to this project will be documented in this file. Dates use ISO 8601 format.

## [Unreleased]
### Fixed
- Explorer RPC client now surfaces RPC errors instead of rendering empty results, improving user feedback and debugging.
- Log file writes flush on warnings and errors to reduce lost diagnostics during crashes while avoiding heavy I/O on info-level logs.

## [v0.1.0-rc] - 2024-12-20 (suggested tag: `v0.1.0-rc`)
### Added
- Cross-platform release workflow that builds, signs, and publishes Linux/macOS/Windows binaries with SHA-256 checksums and detached GPG signatures.
- Web explorer starter (`explorer/`) that queries DRACHMA JSON-RPC for chain tip, blocks, transactions, address balances, and mempool entries.
- Community funding metadata (FUNDING.yml) and expanded security bounty details.

### Changed
- Changelog now mirrors repository history to help downstream packagers track notable merges.

### History references
- 6373013 Merge PR #39: enhance Drachma testnet and mainnet operations.
- 266ea32 Add operational tooling for mainnet and testnet deployments.
- 24dd8db Merge PR #38: documentation improvements for Drachma repo.
- 0d173d0 Enhance documentation for mainnet readiness.
- 4806148 Merge PR #37: expand tests to ~90% coverage.

## v0.1.0-testnet - 2024-11-15
### Added
- Public testnet launch with published genesis, checkpoints, and DNS/bootstrap seeds.
- Multi-node docker-compose harness, faucet, monitoring, and deployment guides.
- Hardened consensus with checkpoint-aware fork resolution, median-time-past, and DoS guards.

## v0.1.0-alpha - 2024-05-01
### Added
- First public alpha with layer1 consensus engine, Schnorr signing, PoW, and initial wallet/GUI.
- Miner reference implementations (CPU/GPU) and basic RPC surface.
