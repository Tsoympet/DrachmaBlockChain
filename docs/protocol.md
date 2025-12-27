# DRACHMA Protocol Specification

## Networks
- **Mainnet magic:** Custom value assigned in consensus parameters.
- **Testnet magic:** Distinct value; permits minimum-difficulty blocks when allowed by timestamp rules.
- **Ports:** Separate defaults for mainnet and testnet for P2P and RPC to avoid accidental cross-talk.

## Block Header
Fields are little-endian where applicable:
- `version` (int32)
- `prev_block` (32-byte hash)
- `merkle_root` (32-byte hash)
- `time` (uint32 epoch seconds)
- `nBits` (compact target)
- `nonce` (uint32)

Hashing uses double SHA-256 over the serialized header. Versionbits signaling is encoded in the version field per deployment settings in consensus parameters.

## Transactions
- Deterministic serialization with tagged SHA-256 transaction IDs.
- Inputs reference previous outputs by hash and index and include a minimal unlocking script.
- Outputs contain a value in atomic units and a locking script.
- Scripts are intentionally limited: push-only data, numeric comparison, and Schnorr signature verification. No loops or recursion are permitted.

## Merkle Tree
Transaction IDs form the leaves. When a layer has an odd count, the final element is duplicated. Internal nodes hash the concatenation of left and right child values using SHA-256d. The root is placed in the block header and verified during block validation.

## Proof-of-Work
- Algorithm: SHA-256d
- Target spacing: 60 seconds
- Retarget window: 60 blocks (3,600 seconds)
- Adjustment clamp: ±25% per period
- Testnet minimum difficulty: allowed when a block timestamp is more than one target spacing behind the previous block; mainnet disallows this shortcut.

Difficulty is stored in compact form (`nBits`). Validation expands the target, checks ranges, and verifies the block hash is below the target.

## Subsidy and Fees
- Initial block subsidy: 50 DRM
- Halving interval: 210,000 blocks
- Supply cap: 42,000,000 DRM enforced by consensus money-range rules
- Fees: the sum of input minus output values within a block are paid to the coinbase transaction.

## Block Validation
1. Check header syntax and proof-of-work target against difficulty rules.
2. Ensure the previous block hash connects to known chain state.
3. Recompute and compare Merkle root.
4. Validate each transaction: correct serialization, signatures, scripts, and money ranges.
5. Enforce coinbase maturity before spends and verify block reward does not exceed subsidy plus fees.
6. Commit state changes atomically; reorg logic rolls back state to the fork point before applying competing branches.

## P2P Messages (Layer 2)
Layer 2 relays headers, blocks, and transactions using deterministic message formats. Messages are authenticated via network magic bytes and length fields. The relay policy includes:
- Fee and size limits for mempool admission
- DoS protections via bounded queues
- Stateless verification for untrusted peers

## RPC Interface (Layer 2)
An authenticated RPC server exposes operational methods such as querying chain height, submitting transactions, and inspecting wallet balances. Authentication uses per-node credentials; no unauthenticated calls are permitted.

## Wallet Behavior
Wallets derive keys from a 24-word mnemonic and store encrypted seeds. Address generation follows deterministic derivation; spending requires Schnorr signatures over transaction digests. Change outputs and fee selection follow mempool relay policy to ensure transactions propagate.

## Cross-Chain (Non-Consensus)
Cross-chain adapters verify external proofs (headers plus Merkle branches) without altering consensus state. Relayers queue messages and present proofs to wallets or monitoring tools. No minted assets or peg-in/peg-out logic exists inside consensus; any bridging is strictly informational.
