# DRACHMA Protocol Overview

This document summarizes the DRACHMA network protocol, block and transaction structures, and Proof-of-Work rules as implemented in the reference stack.

## Network Topology and Messaging

- **Transport:** TCP with length-prefixed messages and optional TLS where configured by services.
- **Handshake:** Version exchange, service flags, network ID (mainnet/testnet), and best-known chain tip.
- **Inventory/Relay:** Peers announce inventories (blocks, transactions). Compact block relay and fee-based transaction admission are handled in Layer 2.
- **Rate Limits:** DoS controls and misbehavior scoring are enforced by services; consensus code remains deterministic and stateless.

## Block Structure (Layer 1)

```
Block {
  Header {
    version           uint32
    prev_block_hash   uint256
    merkle_root       uint256
    time              uint32 (unix, median-time-past constrained)
    bits              uint32 (compact target)
    nonce             uint32
  }
  transactions       vector<Transaction>
}
```

- **Size limits:** Enforced per consensus rules (maximum serialized block size and sigops limits).
- **Merkle root:** Computed over the ordered transaction list. Coinbase is transaction 0 and includes block height and extra nonce.
- **Validation:** Header PoW meets target; timestamps respect median-time-past; chainwork accumulates via target computation.

## Transaction Structure

```
Transaction {
  version    int32
  inputs     vector<TxIn>
  outputs    vector<TxOut>
  lock_time  uint32
}

TxIn {
  prevout { txid uint256, index uint32 }
  script_sig   bytes
  sequence     uint32
}

TxOut {
  value        int64 (satoshis equivalent)
  script_pubkey bytes
}
```

- **Signature rules:** Script execution follows consensus-defined opcode set; scripts are validated in Layer 1.
- **Locktime/sequence:** Relative and absolute locktimes enforced per consensus rules; mempool may apply stricter policy.
- **Fee calculation:** Inputs minus outputs; minimum relay and mempool policies live in services.

## Proof of Work

- **Hashing:** Double SHA-256 over the serialized block header.
- **Target encoding:** `bits` uses compact encoding; Layer 1 recalculates the target for the next block per difficulty adjustment parameters defined in the codebase (see `layer1-core` difficulty module).
- **Validation:** A block is valid if `double_sha256(header) <= target` and all other consensus checks pass.
- **Work templates:** Layer 2 exposes RPC endpoints to retrieve block templates for miners. Miners modify `nonce` and extra nonce in coinbase; timestamp updates are permitted within validity bounds.

## Mempool and Relay (Layer 2)

- **Admission:** Checks fee rate, standardness of scripts, and ancestor/descendant limits before relaying to peers.
- **Propagation:** Inv/getdata/tx flows, compact block relay for blocks. Transactions are rebroadcast periodically to maintain network health.
- **Fee estimation:** Rolling estimator based on confirmed transactions; feeds wallet construction.

## Wallet and Addresses (Layer 2)

- HD wallet with deterministic key derivation (BIP32-style hierarchy).
- Address formats follow the repository's selected scheme (base58/bech32 equivalent as implemented under wallet modules).
- Wallets construct transactions referencing Layer 1 consensus rules and submit via RPC to P2P relay.

## Light Clients and Indexing

- Layer 2 may expose lightweight proofs (headers-first synchronization, transaction lookups) using indices built from chain data.
- Merkle proofs allow verification of transaction inclusion without full blocks.

## Extensibility and Compatibility

- **Consensus changes:** Require activation mechanisms (height- or time-based) and broad review. Services and UI must remain compatible across activations.
- **Non-consensus changes:** RPC additions or mempool policy tweaks do not affect historical validity but should maintain backward compatibility.

## References in Codebase

- **Consensus/Validation:** `layer1-core/` modules for block/transaction checks, difficulty calculation, and chainstate.
- **Networking/Policy/RPC:** `layer2-services/` for P2P, mempool, fee estimation, and RPC handlers.
- **Wallet/Keys:** Wallet backends and key management under `layer2-services/`.
- **Miners:** Reference CPU/GPU miners under `miners/` consuming RPC work templates.
