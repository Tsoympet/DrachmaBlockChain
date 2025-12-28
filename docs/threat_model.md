# Threat Model

This document summarizes the primary assets, trust boundaries, and threats relevant to DRACHMA mainnet readiness. It highlights how mitigations should be validated through fuzzing, integration tests, and operational monitoring.

## Assets
- **Consensus state:** Canonical chain, block index, UTXO set, mempool contents.
- **Keys and secrets:** Wallet seeds, private keys, signing nonces, RPC authentication tokens.
- **Networking identity:** Node keys, peer connection metadata, ban lists, and peer diversity sets.
- **User funds and privacy:** On-chain balances, transaction graph privacy, and metadata leaked via P2P queries.

## Trust Boundaries
- **P2P network:** Untrusted; all inbound/outbound peers can be malicious, Sybil, or eclipsing attackers.
- **RPC/API surface:** Authenticated but potentially exposed to misconfiguration; treat inputs as untrusted.
- **Build/release pipeline:** Trusted when reproducible and verified; artifacts must be signed and hash-checked.
- **Third-party dependencies:** Treat external libraries and container base images as untrusted until verified via SBOM and CVE review.

## Threats and Mitigations
### Consensus and Validation
- **Invalid block/transaction propagation:** Mitigate via strict validation, contextual checks, and fuzz targets for parsers and script engines.
- **Reorg manipulation:** Use checkpoints, finality heuristics, and reorg stress tests to ensure deterministic chain selection under partitions.
- **Resource exhaustion:** Enforce per-peer and global limits on block/tx size, signature verification batches, and mempool growth; integrate libFuzzer/ASan/UBSan to catch parser overflows.

### Networking Attack Vectors
- **DoS floods and dust attacks:** Apply score-based banning, dust filters, per-IP/subnet connection caps, and transaction relay bloom filters to bound resource use.
- **Eclipse/Sybil:** Diversify outbound peers (AS-level/subnet diversity, Tor/i2p mixing), rotate addresses, and verify multiple bootstrap sources.
- **Protocol misuse:** Validate message ordering/state (e.g., handshake before tx relay), reject malformed or unexpected messages, and fuzz message deserialization.

### Privacy and Identity
- **Address linking and fingerprinting:** Use bloom-filtered SPV support, randomized nonce salts, and optional onion routing (Tor/i2p) for inbound services.
- **Metadata leaks:** Limit debug/log verbosity by default, avoid persistent identifiers across sessions, and consider Dandelion-like relay paths where applicable.

### Wallet and Key Handling
- **Key exfiltration:** Encrypt keystore at rest, minimize key lifetime in memory, and lock wallets automatically after inactivity. Ensure RPC auth uses strong credentials and TLS where supported.
- **Backup compromise:** Provide deterministic backup/restore flows and encourage hardware wallet isolation; document secure wipe procedures for compromised hosts.

### Supply Chain
- **Dependency compromise:** Maintain CycloneDX SBOMs, monitor advisories, and pin deterministic versions; require signatures for release artifacts and container images.
- **Build integrity:** Enforce reproducible builds, signed tags, and isolated CI runners with minimal secrets exposure.

## Validation Strategy
- **Fuzzing:** Continuous libFuzzer runs (block parsing, tx validation, P2P message codecs, Schnorr batch verification) with ASan/UBSan; track coverage and triage crashes before release.
- **Integration and stress tests:** Multi-node regtest scenarios for reorgs, partitions, and high-load IBD; monitor metrics for memory/CPU/network saturation and behavior under mining races.
- **Monitoring and alerting:** Capture anomalous peer behavior, ban score spikes, and validation errors; feed into dashboards and pager rotation.

## Residual Risks
Document any unmitigated issues (e.g., reliance on external DNS seeds, remaining unauthenticated transport, or third-party library exposure) with owner and remediation timeline.

Keep this threat model updated alongside major protocol or networking changes, and review it before each release candidate.
