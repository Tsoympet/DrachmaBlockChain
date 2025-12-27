# DRACHMA Blockchain

# DRACHMA Blockchain (DRM)

DRACHMA is a **Proof-of-Work monetary blockchain** designed for long-term stability, auditability, and minimal trust assumptions.

The project focuses on:
- deterministic consensus rules,
- conservative cryptographic design,
- strict separation of system layers,
- and transparent network launch conditions.

This repository contains the **reference implementation** of the DRACHMA network.

---

## Core Principles

- **Proof-of-Work:** SHA-256d (double SHA-256), unmodified  
- **Launch Model:** No premine, no privileged rewards, no special launch logic  
- **Supply Cap:** 42,000,000 DRM  
- **Consensus First:** All critical rules reside exclusively in Layer 1  
- **No Governance Logic:** No voting systems, no administrative keys  
- **No Smart Contracts:** Monetary network only  

Network neutrality is achieved through **absence of privilege**, not through special enforcement mechanisms.

---

## Architecture Overview

The system is divided into **three strictly separated layers**:

### Layer 1 — Core Blockchain (Consensus-Critical)
Location: `layer1-core/`

Responsibilities:
- Block and transaction validation  
- Proof-of-Work (SHA-256d)  
- Difficulty adjustment  
- UTXO accounting  
- Merkle tree construction  
- Schnorr signature verification  
- Persistent storage (blocks and chainstate)  

Changes in this layer require extreme caution and full review.

---

### Layer 2 — Services (Non-Consensus)
Location: `layer2-services/`

Responsibilities:
- Peer-to-peer networking  
- RPC interface  
- Mempool management and fee policy  
- Wallet backend services  
- Transaction indexing  
- Cross-chain interoperability (proof-based, non-consensus)  

This layer must never alter consensus rules.

---

### Layer 3 — Desktop Application
Location: `layer3-app/`

Responsibilities:
- Graphical desktop application  
- Wallet interface  
- Miner control  
- User-facing configuration  
- Asset loading (icons, documentation, legal text)  

This layer contains no consensus logic.

---

## Launch Characteristics

The network launches without:
- pre-allocation of supply,  
- privileged mining phases,  
- protocol-enforced launch conditions,  
- or embedded checkpoints.

Mining and block production begin normally from the first block.

Launch conditions are documented in:


---

## Cryptography

- **Proof-of-Work Hashing:** SHA-256d  
- **Merkle Trees:** SHA-256d  
- **Transaction Hashing:** Tagged SHA-256  
- **Digital Signatures:** Schnorr (secp256k1)  

Legacy signature schemes are intentionally excluded.

---

## Mining

Reference mining implementations are provided:
- CPU miner  
- GPU miner (CUDA)  
- GPU miner (OpenCL)  

These miners serve as transparent reference tools and are not optimized black-box software.

---

## Repository Structure

drachma-blockchain/
├─ layer1-core/ # Consensus-critical logic
├─ layer2-services/ # Networking, RPC, wallet backend, cross-chain
├─ layer3-app/ # Desktop application
├─ miners/ # Reference CPU & GPU miners
├─ testnet/ # Test network parameters
├─ docs/ # Technical documentation
├─ tests/ # Test skeletons
├─ common/ # Shared utilities
└─ scripts/ # Build and run scripts


---

## Status

This repository currently contains a **complete structural skeleton** with placeholders.  
Implementation is intended to be added incrementally with full review of consensus-critical components.

---

## License

This project is released under the MIT License.  
See `LICENSE` for details.

