# DRACHMA Threat Model

## Assets
- Chain state integrity (blocks, headers, UTXO set)
- Monetary policy (supply cap, halving schedule)
- Wallet private keys and encrypted seeds
- Network availability for block and transaction relay

## Adversaries
- External attackers with significant hash power attempting double-spend or reorg attacks.
- Network-level adversaries capable of partitioning or eclipsing nodes.
- Malicious peers flooding invalid data to exhaust resources.
- Malware targeting wallet storage to exfiltrate keys.

## Assumptions
- Honest majority of cumulative proof-of-work over time.
- Secure local environment for wallet storage; passphrases remain secret.
- Nodes perform full validation of all headers, blocks, and transactions without trusting peers.
- Cryptographic primitives (SHA-256d, secp256k1 Schnorr) remain unbroken.

## Mitigations
- Deterministic validation: Every block is fully verified (PoW, scripts, Merkle roots, money ranges) before acceptance.
- Difficulty retarget clamping limits sudden drops in work threshold, reducing window for cheap long-range reorgs.
- DoS controls in mempool and networking (bounded queues, authenticated RPC, fee/size policies) limit resource exhaustion.
- UTXO database uses atomic updates per block and rollback metadata for reorg safety.
- Wallet seeds are encrypted with AES-256 and never transmitted; UI does not expose raw keys.
- No admin keys or privileged consensus paths exist; all participants follow the same rules.

## Residual Risks
- A majority hash power adversary can still execute deep reorgs and double spends.
- Users connecting through untrusted networks may be susceptible to eclipse if peers are limited; running multiple connections and DNS seeds mitigates but does not eliminate this.
- Endpoint security remains critical; compromised hosts can leak wallet material regardless of protocol safeguards.

## Operational Guidance
- Run multiple diverse peers and verify connection freshness.
- Regularly back up encrypted wallet data and mnemonic seeds offline.
- Keep software updated; validate binaries from trusted sources and verify signatures where available.
