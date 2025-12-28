# Sidechain Smart Contracts

The DRACHMA sidechain offers an **optional, merge-mined execution layer** with a constrained EVM designed for auditability:

- ~50 essential opcodes, omitting risky calls such as `SELFDESTRUCT` and `DELEGATECALL`.
- Schnorr signature verification alongside ECDSA for transactions and precompiles.
- Gas is paid in wrapped DRM (**wDRM**) minted via the mainnet↔sidechain peg.
- Keccak-256 hashing via OpenSSL and deterministic gas metering tied to DRM peg fees.

## Using the Desktop GUI

1. Enable the sidechain in **Settings → Sidechain support**. Provide the RPC endpoint (default `http://localhost:8545`).
2. Open the **Sidechain** tab to see DRM and wDRM balances plus sync/peer status.
3. Use the **Bridge** buttons to lock DRM → wDRM or burn wDRM → DRM through the peg.
4. In **Smart Contracts**, paste an ABI JSON or choose ERC-20/721 presets, enter the contract address, then **Call** for reads or **Send transaction** for writes.
5. **NFTs** shows owned tokens with transfer/mint shortcuts (tied to the ERC-721-like precompile).
6. The **dApps** browser (QWebEngine) opens a local gateway such as `http://localhost:8080`; shortcuts to common explorers/DEX demos are provided.

## Deploying Contracts

Deploy contracts with standard Solidity tooling pointed at the sidechain RPC. Example (Solidity-compatible) contract:

```solidity
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.21;

contract Greeting {
    string private message = "hello";
    function greet() external view returns (string memory) { return message; }
    function setGreeting(string calldata m) external { message = m; }
}
```

After deployment, copy the contract address into the GUI, load the ABI, and interact via the generated function grid.

## Tooling & Explorer

- **Explorer:** run or open the sidechain explorer listed in `explorer/` or your dApp gateway.
- **SDK:** standard Web3.js/ethers.js providers work against the sidechain RPC; prefer HTTPS or IPC where available.
- **Security:** contracts are isolated to the sidechain. Mainnet DRM remains minimal and unaffected.
