# PARTHENON CHAIN Asset Directory

**Theme**: Classical Greece · Value · Order · Timelessness  
**Visual DNA**: Marble, Bronze, Silver, Obsidian

This directory contains all visual assets and branding materials for the PARTHENON CHAIN blockchain project.

## Directory Structure

### `/core-icons/`
Application branding icons used by the desktop wallet:
- `app-icon.svg` - Main application icon (128x128) - Classical architecture inspired
- `tray-icon.svg` - System tray icon (32x32) - Parthenon symbol
- `splash-icon.svg` - Splash screen icon (256x256) - Marble and bronze theme

### `/icons/`
Asset type icons representing the three core assets:
- `asset-tln.svg` - TLN (Talon) coin icon - **Bronze** (#CD7F32)
- `asset-drm.svg` - DRM (Drachma) coin icon - **Silver** (#C0C0C0)
- `asset-obl.svg` - OBL (Obol) coin icon - **Aegean Blue** (#1E4D8B)

### `/nft-icons/`
NFT category icons for the sidechain marketplace:
- `nft-default.svg` - Default fallback for uncategorized NFTs
- `nft-hero.svg` - Hero category (Classical heroes)
- `nft-monument.svg` - Monument category (Greek architecture)
- `nft-mythology.svg` - Mythology category (Gods and legends)
- `nft-symbol.svg` - Symbol category (Classical symbols)

### `/ui-icons/`
User interface control icons (with light and dark variants):
- Network status: `network.svg`, `error.svg`, `warning.svg`, `shield.svg`
- Wallet: `wallet.svg`, `address-book.svg`, `key.svg`, `qr.svg`
- Transactions: `transactions.svg`, `tx-pending.svg`, `tx-out.svg`
- Staking: `stake-inactive.svg`, `validator.svg`, `rewards.svg`
- Mining: `hash.svg`, `difficulty.svg`
- System: `disk.svg`

Dark variants are in `/ui-icons/dark/` subdirectory.

## Asset Guidelines

### Icon Format
- **Format:** SVG (Scalable Vector Graphics)
- **Size:** Icons should be designed at their intended display size
- **Colors:** PARTHENON CHAIN Classical Greek theme
  - **Marble White**: #F5F5F0 (Pentelic marble)
  - **Obsidian Black**: #1C1C1E (Volcanic strength)
  - **Bronze**: #CD7F32 (Ancient Greek statuary)
  - **Silver**: #C0C0C0 (Laurion silver mines)
  - **Aegean Blue**: #1E4D8B (Mediterranean)

### Adding New Icons
1. Create SVG file with descriptive name
2. Place in appropriate subdirectory
3. Update this README with description
4. Test icon rendering in wallet UI
5. Follow Classical Greek theme
6. Commit with clear description

### Replacing Icons
To replace an icon while maintaining compatibility:
1. Keep the same filename
2. Maintain the same viewBox dimensions
3. Test in both light and dark themes
4. Ensure accessibility (sufficient contrast)
5. Follow marble, bronze, silver, obsidian color palette

## Usage in Code

Icons are loaded at runtime by the Qt wallet application. The application references these paths directly:

```cpp
// Example from layer3-app/qt/main.cpp
QIcon appIcon(":/assets/core-icons/app-icon.svg");
QPixmap assetIcon(":/assets/icons/asset-drm.svg");
```

Do not rename or move files without updating the corresponding UI code.

## Asset Metadata

The `asset_metadata.json` file contains metadata about asset properties and display settings. This is used by the wallet to correctly render asset information with Classical Greek theme.

## Notes

- Icons should work well on both light and dark backgrounds
- SVG format allows scaling without quality loss
- Keep file sizes reasonable (<50KB per icon)
- Icons should be recognizable at small sizes (16x16, 24x24)
- All icons are loaded from the filesystem; no embedded binary resources
- Classical Greek architectural motifs (columns, pediments, friezes)
- Marble texture for primary surfaces, bronze/silver for accents

## License

All icons in this directory are part of the PARTHENON CHAIN project and follow the project's MIT license. See the root LICENSE file for details.
