# DRACHMA Core Desktop quick guide

- **Security first**: Your wallet file is AES-256 encrypted. Always set a strong passphrase and keep offline backups.
- **Recovery seed**: Open **Settings → Show recovery seed** to generate a BIP39 mnemonic. Write the 12 words on paper and store them offline. Use **Restore from seed** to rebuild the hot wallet on a new machine.
- **Hardware wallets**: Click **Detect hardware wallet** to verify Ledger/Trezor visibility (requires `libhidapi`). Keep devices unlocked and connected via USB.
- **QR flows**: On the **Send** tab, use **Scan QR** to pull a recipient from an image or webcam capture file. On **Receive**, share the rendered QR for your address.
- **Fees and history**: Adjust the fee slider or custom sat/vB to target confirmation speed. Filter transactions by direction, status, or search text in **Transactions**.
- **Internationalization**: The app attempts to load `assets/i18n/drachma_<locale>.qm`; place translations there and restart.
- **Network health**: Watch the sync bar and RPC indicator on Overview. Any RPC degradation triggers a blocking alert so you can retry or reconfigure connectivity.
