# Verifying Downloads

**Critical Security Practice**: Always verify downloaded binaries before running them. This ensures you're running authentic, unmodified PARTHENON CHAIN software.

## Table of Contents

- [Why Verify?](#why-verify)
- [Quick Verification](#quick-verification)
- [Detailed Verification Steps](#detailed-verification-steps)
- [GPG Signature Verification](#gpg-signature-verification)
- [Reproducible Builds](#reproducible-builds)
- [Troubleshooting](#troubleshooting)

## Why Verify?

Verifying downloads protects you from:
- **Corrupted downloads**: Network errors can corrupt files
- **Malicious modifications**: Attackers could replace binaries with trojans
- **Man-in-the-middle attacks**: Compromised mirrors or DNS hijacking

By verifying checksums and signatures, you ensure the software you download is exactly what the developers released.

## Quick Verification

### Step 1: Download both the binary and checksum

```bash
# Example for Linux
wget https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/v0.1.0/parthenon-core-v0.1.0-linux-x86_64.tar.gz
wget https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/v0.1.0/parthenon-core-v0.1.0-linux-x86_64.tar.gz.sha256
```

### Step 2: Verify the checksum

**Linux/macOS:**
```bash
sha256sum -c parthenon-core-v0.1.0-linux-x86_64.tar.gz.sha256
```

Expected output:
```
parthenon-core-v0.1.0-linux-x86_64.tar.gz: OK
```

**Windows PowerShell:**
```powershell
$expectedHash = (Get-Content parthenon-core-v0.1.0-win-x86_64.zip.sha256).Split()[0]
$actualHash = (Get-FileHash parthenon-core-v0.1.0-win-x86_64.zip -Algorithm SHA256).Hash
if ($expectedHash -eq $actualHash) {
    Write-Host "Checksum verified: OK" -ForegroundColor Green
} else {
    Write-Host "Checksum verification FAILED!" -ForegroundColor Red
}
```

If verification passes, you can safely extract and use the binaries.

## Detailed Verification Steps

### 1. Download Release Files

From the [GitHub releases page](https://github.com/Tsoympet/PARTHENON-CHAIN/releases), download:

1. The binary archive (`.tar.gz` or `.zip`)
2. The corresponding `.sha256` checksum file
3. The GPG signature file (`.asc`) if available

Example:
```
parthenon-core-v0.1.0-linux-x86_64.tar.gz
parthenon-core-v0.1.0-linux-x86_64.tar.gz.sha256
parthenon-core-v0.1.0-linux-x86_64.tar.gz.asc  (if available)
```

### 2. Verify SHA256 Checksum

The checksum ensures the file hasn't been corrupted or tampered with.

**On Linux:**
```bash
# Automatic verification
sha256sum -c parthenon-core-v0.1.0-linux-x86_64.tar.gz.sha256

# Manual verification
sha256sum parthenon-core-v0.1.0-linux-x86_64.tar.gz
# Compare output with contents of .sha256 file
cat parthenon-core-v0.1.0-linux-x86_64.tar.gz.sha256
```

**On macOS:**
```bash
# Automatic verification
shasum -a 256 -c parthenon-core-v0.1.0-macos-arm64.tar.gz.sha256

# Manual verification
shasum -a 256 parthenon-core-v0.1.0-macos-arm64.tar.gz
cat parthenon-core-v0.1.0-macos-arm64.tar.gz.sha256
```

**On Windows (PowerShell):**
```powershell
# Calculate hash
$hash = (Get-FileHash parthenon-core-v0.1.0-win-x86_64.zip -Algorithm SHA256).Hash

# Display for comparison
Write-Host "Actual:  " $hash
Write-Host "Expected:" (Get-Content parthenon-core-v0.1.0-win-x86_64.zip.sha256)
```

The two hashes must match exactly. If they don't match:
- **DO NOT** run the binary
- Re-download from the official source
- If problem persists, report it on GitHub

### 3. Verify Individual Binaries (After Extraction)

After extracting the archive, you can verify individual binaries:

```bash
# Extract
tar -xzf parthenon-core-v0.1.0-linux-x86_64.tar.gz

# Verify binaries against included SHA256SUMS
sha256sum -c SHA256SUMS
```

Expected output:
```
drachmad: OK
drachma-cli: OK
drachma_cpu_miner: OK
```

## GPG Signature Verification

GPG signatures provide stronger authentication than checksums alone. They prove the release was created by someone with access to the official signing key.

### Setting Up GPG

**Install GPG:**
```bash
# Ubuntu/Debian
sudo apt-get install gnupg

# macOS
brew install gnupg

# Windows
# Download from https://www.gnupg.org/download/
```

### Import the Official Signing Key

```bash
# Import from keyserver
gpg --keyserver hkps://keys.openpgp.org --recv-keys <KEY_ID>

# Or import from project repository
curl https://github.com/Tsoympet/PARTHENON-CHAIN/raw/main/contrib/signing-keys/release.asc | gpg --import

# IMPORTANT: Verify key fingerprint matches official sources
gpg --fingerprint <KEY_ID>
# Compare the fingerprint with what's published on:
# - GitHub repository README
# - Official website
# - Release announcements
# - Multiple independent sources
```

**Important:** Always verify the key fingerprint through multiple independent channels:
- GitHub repository
- Official website
- Social media (official accounts)
- Other developers

### Verify the Signature

```bash
# Verify release signature
gpg --verify parthenon-core-v0.1.0-linux-x86_64.tar.gz.asc parthenon-core-v0.1.0-linux-x86_64.tar.gz
```

Expected output:
```
gpg: Signature made [DATE]
gpg:                using RSA key [KEY_ID]
gpg: Good signature from "PARTHENON CHAIN Release Signing Key"
```

**Warning signs:**
- "BAD signature" - **DO NOT USE** the binary
- "Can't check signature: No public key" - Import the signing key first
- "This key is not certified" - Normal for first use; verify fingerprint manually

## Reproducible Builds

For maximum security, you can build PARTHENON CHAIN from source and verify your build matches the official release.

### Prerequisites

- Linux system (Ubuntu 20.04+ recommended)
- Build tools: `cmake`, `gcc`, `git`
- Dependencies: OpenSSL, Boost, LevelDB

### Build from Source

```bash
# Clone repository
git clone https://github.com/Tsoympet/PARTHENON-CHAIN.git
cd PARTHENON-CHAIN

# Checkout the release tag
git checkout v0.1.0

# Run reproducible build script
./scripts/reproducible-build.sh

# Compare your build with official release
sha256sum dist/parthenon-core-v0.1.0-linux-x86_64.tar.gz
# Should match the official .sha256 file
```

If the checksums match, you've independently verified that the source code produces the exact same binary as the official release.

See [Reproducible Builds Guide](./reproducible-builds.md) for detailed instructions.

## Verifying Using Gitian

For advanced users, Gitian builds provide the highest level of reproducibility:

```bash
# Setup Gitian environment (one-time setup)
git clone https://github.com/devrandom/gitian-builder.git
cd gitian-builder

# Build using Gitian descriptor
./bin/gbuild ../PARTHENON-CHAIN/contrib/gitian-descriptors/gitian-linux.yml

# Compare signatures with other builders
# Multiple independent builders should produce identical hashes
```

See `contrib/gitian-descriptors/README.md` for Gitian build instructions.

## Best Practices

1. **Always download from official sources:**
   - GitHub releases: https://github.com/Tsoympet/PARTHENON-CHAIN/releases
   - Official website (if available)
   - Never use third-party download sites

2. **Verify over multiple channels:**
   - Check checksums
   - Verify GPG signatures (when available)
   - Compare with community-reported hashes

3. **Use HTTPS:**
   - Ensure you're downloading over HTTPS
   - Check for valid SSL certificate

4. **Keep GPG keys updated:**
   - Periodically refresh signing keys
   - Watch for key rotation announcements

5. **Report suspicious downloads:**
   - If verification fails, report it immediately
   - Never run unverified binaries

## Troubleshooting

### "No such file or directory" when verifying

Make sure:
- The archive file and .sha256 file are in the same directory
- You're running the command from that directory
- Filenames match exactly (case-sensitive)

### Checksum doesn't match

1. Re-download both files
2. Check you downloaded the right version/platform
3. If still failing, report on GitHub Issues

### "Can't check signature: No public key"

Import the signing key first:
```bash
gpg --keyserver keys.openpgp.org --recv-keys <KEY_ID>
```

### GPG signature expired

Check for updated keys:
- GitHub repository: `contrib/signing-keys/`
- Release announcements
- Developer communications

## FAQ

**Q: Is checksum verification enough?**
A: Checksums protect against corruption and some attacks, but GPG signatures provide stronger authentication. Use both when available.

**Q: What if GPG signature isn't available?**
A: Some releases may not have GPG signatures yet. At minimum, always verify checksums. Consider building from source for maximum assurance.

**Q: Can I trust checksums downloaded from GitHub?**
A: GitHub uses HTTPS and has good security practices. For maximum paranoia, verify checksums through multiple channels (social media, forums, etc.)

**Q: How often should I verify?**
A: Verify every download, every time. Never skip verification.

**Q: What if I already ran an unverified binary?**
A: If you suspect compromise:
1. Stop the software immediately
2. Run antivirus scan
3. Consider the system potentially compromised
4. Move funds to a new wallet (from a clean system)

## Security Contact

If you discover a security issue with releases or the verification process:
- **DO NOT** post publicly
- Email: security@parthenon-chain.org (if available)
- Or create a private security advisory on GitHub

## Additional Resources

- [Installation Guide](./install.md)
- [Reproducible Builds](./reproducible-builds.md)
- [Security Best Practices](../security/security-overview.md)
- [GitHub Releases](https://github.com/Tsoympet/PARTHENON-CHAIN/releases)

## License

PARTHENON CHAIN is released under the MIT License. See LICENSE file for details.
