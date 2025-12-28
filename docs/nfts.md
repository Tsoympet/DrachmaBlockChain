# NFTs on the DRACHMA Sidechain

The sidechain ships an ERC-721-like NFT precompile backed by LevelDB trie storage. Gas is paid in **wDRM** and ownership is enforced by the sidechain consensus rules.

## Viewing and Transferring in the Desktop App

1. Enable sidechain support and open the **Sidechain → NFTs** section.
2. The gallery lists token ID, name, and metadata URL. Images are fetched by dApps; the wallet keeps metadata lightweight.
3. Select a token and click **Transfer** to send it to another address. Confirm the recipient in the dialog.
4. If your configured minting contract allows it, use **Mint** to request a new token.

## Metadata

- Prefer IPFS or other content-addressed URIs (e.g., `ipfs://...`).
- Include standard fields: `name`, `description`, `image`, and optional `attributes`.
- Keep assets small; large media should be served via gateways/CDNs managed by the dApp.

## Bridge and Security Notes

- NFTs remain on the sidechain; only wDRM crosses the peg bridge.
- Duplicate mints are rejected by the precompile. Anti-replay and timeout rules protect bridge flows.
- Mainnet DRM remains unaffected if you disable the sidechain in Settings.
