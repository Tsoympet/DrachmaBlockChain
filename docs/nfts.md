# NFTs (TLN, asset_id=0)

NFT minting, ownership, and transfers are bound to TLN only. The WASM validator rejects NFT calls funded by DRM or OBL.

## Model
- RPC entrypoints: `mint_nft` and `transfer_nft` (see `sidechain/rpc/wasm_rpc.*`).
- State isolation: ownership and metadata hashes are stored under the NFT domain; other domains cannot access them.
- Gas: fixed-cost metering for mint/transfer, paid in TLN.

## Using the Desktop GUI
1. Open **Sidechain → NFTs**. Balances show TLN (NFTs), DRM (contracts), and OBL (dApps).
2. Use **Mint** to register a token ID + metadata hash; the wallet funds TLN automatically.
3. Use **Transfer** to move ownership; the UI will reject mixed-asset attempts.

## Tips
- Keep metadata URIs deterministic and hash them client-side before submission.
- Use the TLN domain only; bridging or wrapping assets for NFTs is disallowed by consensus.
