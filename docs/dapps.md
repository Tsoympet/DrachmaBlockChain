# Building dApps on the DRACHMA Sidechain

The merge-mined sidechain supports web3-compatible dApps with gas in wDRM and a restricted opcode set for safety.

## Developing

- Point your Web3.js/ethers.js provider to the sidechain RPC (default `http://localhost:8545`).
- Use Solidity contracts compiled for EVM; avoid unsupported opcodes such as `SELFDESTRUCT` and `DELEGATECALL`.
- Test against the ERC-721 precompile or the peg bridge for wrapped DRM flows.

### Example dApp (DEX sketch)

- Smart contract: constant-product AMM for wDRM/token pairs.
- Frontend: React/Vue served at `http://localhost:8080/dex` with MetaMask/Web3Modal pointed at the sidechain RPC.
- Security: validate slippage client-side; surface gas estimations in wDRM.

## Using the Built-in dApp Browser

1. Enable the sidechain in Settings and set the dApp gateway URL.
2. Open **Sidechain → dApps**. The browser loads the default gateway and lists shortcuts (explorer, DEX demo, NFT marketplace).
3. Click a shortcut or enter a URL; the embedded WebEngine view will open it and reuse the configured provider endpoint.

## Deployment Tips

- Serve static assets over HTTPS where possible; local development may use `http://localhost:8080`.
- Keep RPC exposed only to trusted networks; prefer IPC or authenticated HTTP.
- Publish ABIs alongside contracts so users can load them in the wallet's Smart Contracts section.
