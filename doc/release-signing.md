# Release Signing Guide

## Overview

All PARTHENON CHAIN releases must be cryptographically signed to ensure authenticity and integrity. This guide explains how to sign releases and verify signatures.

## Why Sign Releases?

Signing releases protects users from:
- **Malicious binaries**: Attackers cannot distribute backdoored versions
- **Man-in-the-middle attacks**: Downloads cannot be tampered with in transit
- **Supply chain attacks**: Compromised mirrors cannot serve fake releases

## Prerequisites

### For Maintainers (Signing)

- GPG key with signing capability
- Access to the release signing key (kept offline/secure)
- GitHub repository write access

### For Users (Verifying)

- GPG installed (`gpg --version`)
- Maintainer's public key imported

## Maintainer Signing Process

### 1. Set Up GPG Key

```bash
# Generate a new GPG key (if you don't have one)
gpg --full-generate-key

# Use these settings:
# - Key type: RSA and RSA
# - Key size: 4096 bits
# - Expiration: 2 years (renewable)
# - Real name: Your Name
# - Email: your@email.com

# Export public key
gpg --armor --export your@email.com > parthenon-signing-key.asc

# Publish to keyserver
gpg --keyserver keys.openpgp.org --send-keys YOUR_KEY_ID
```

### 2. Sign Release Tag

```bash
# Create and sign a release tag
git tag -s v0.1.0 -m "PARTHENON CHAIN v0.1.0"

# Verify the tag signature
git tag -v v0.1.0

# Push the signed tag
git push origin v0.1.0
```

### 3. Sign Release Binaries

```bash
# After building the release
cd dist

# Sign the archive
gpg --armor --detach-sign drachma-v0.1.0-linux-x86_64.tar.gz

# This creates drachma-v0.1.0-linux-x86_64.tar.gz.asc

# Sign the checksums file
gpg --armor --detach-sign drachma-v0.1.0-linux-x86_64.tar.gz.sha256
```

### 4. Create Signed Checksums

```bash
# Generate checksums
sha256sum drachma-v0.1.0-*.tar.gz > SHA256SUMS

# Sign the checksums file
gpg --clearsign SHA256SUMS

# This creates SHA256SUMS.asc with embedded signature
```

### 5. Upload to GitHub Release

Upload these files to the GitHub release:
- `drachma-v0.1.0-linux-x86_64.tar.gz` (binary archive)
- `drachma-v0.1.0-linux-x86_64.tar.gz.sha256` (checksum)
- `drachma-v0.1.0-linux-x86_64.tar.gz.asc` (GPG signature)
- `SHA256SUMS.asc` (signed checksums)
- `parthenon-signing-key.asc` (public key)

## User Verification Process

### 1. Import Maintainer's Public Key

```bash
# Download from GitHub release
wget https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/v0.1.0/parthenon-signing-key.asc

# Import the key
gpg --import parthenon-signing-key.asc

# Verify key fingerprint (should be published on website/README)
gpg --fingerprint
```

### 2. Verify Release Tag

```bash
# Clone repository
git clone https://github.com/Tsoympet/PARTHENON-CHAIN.git
cd PARTHENON-CHAIN

# Verify the tag signature
git tag -v v0.1.0
```

You should see:
```
gpg: Signature made [date]
gpg: Good signature from "PARTHENON CHAIN Release Signing Key"
```

### 3. Verify Binary Signature

```bash
# Download release and signature
wget https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/v0.1.0/drachma-v0.1.0-linux-x86_64.tar.gz
wget https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/v0.1.0/drachma-v0.1.0-linux-x86_64.tar.gz.asc

# Verify signature
gpg --verify drachma-v0.1.0-linux-x86_64.tar.gz.asc drachma-v0.1.0-linux-x86_64.tar.gz
```

Expected output:
```
gpg: Signature made [date]
gpg: Good signature from "PARTHENON CHAIN Release Signing Key"
```

### 4. Verify Checksum

```bash
# Download checksum file
wget https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/v0.1.0/drachma-v0.1.0-linux-x86_64.tar.gz.sha256

# Verify checksum
sha256sum -c drachma-v0.1.0-linux-x86_64.tar.gz.sha256
```

Expected output:
```
drachma-v0.1.0-linux-x86_64.tar.gz: OK
```

### 5. Complete Verification

All three checks should pass:
1. ✅ GPG signature is valid
2. ✅ Signature from trusted key
3. ✅ Checksum matches

Only then should you extract and run the binary.

## Automated GitHub Actions Signing

### Set Up Repository Secrets

1. Export private key (secure location only!):
```bash
gpg --armor --export-secret-keys your@email.com > private-key.asc
```

⚠️ **CRITICAL SECURITY WARNING**: This file contains your private key in plaintext!
- Never commit this file to version control
- Keep it only in secure, encrypted storage
- Delete immediately after use
- Consider using a subkey instead of your master key

2. Add to GitHub repository secrets:
   - Navigate to: Settings → Secrets → Actions
   - Add secret: `GPG_PRIVATE_KEY` = contents of `private-key.asc`
   - Add secret: `GPG_PASSPHRASE` = your key passphrase (if set)

3. **IMPORTANT - Delete immediately**:
```bash
# Securely delete the private key file
shred -vfz -n 10 private-key.asc
# Or on macOS
srm -v private-key.asc
```

**Best Practice**: Use a dedicated signing subkey for CI/CD, not your master key.

### Workflow Integration

The release workflow (`.github/workflows/release.yml`) automatically signs releases when `GPG_PRIVATE_KEY` is configured.

## Key Management Best Practices

### For Project Maintainers

1. **Offline Master Key**
   - Keep master signing key on air-gapped machine
   - Use subkeys for day-to-day signing
   - Store offline backup in secure location

2. **Key Rotation**
   - Set expiration date (recommend 2 years)
   - Extend before expiration or rotate
   - Announce key changes publicly

3. **Revocation Certificate**
   - Generate immediately after creating key
   - Store separately from key
   - Use if key is compromised

```bash
# Generate revocation certificate
gpg --output revoke.asc --gen-revoke your@email.com
```

4. **Multiple Maintainers**
   - Each maintainer has own signing key
   - All keys published in repository
   - Require signatures from multiple maintainers for critical releases

### Key Fingerprint Publication

Publish the key fingerprint in multiple locations:
- **README.md** in repository
- **Project website** (https://drachma.org)
- **GitHub release notes**
- **Social media** (Twitter, etc.)

Example fingerprint format:
```
PARTHENON CHAIN Release Signing Key
Fingerprint: 1234 5678 90AB CDEF 1234  5678 90AB CDEF 1234 5678
```

## Troubleshooting

### "gpg: Can't check signature: No public key"

Import the maintainer's public key:
```bash
gpg --keyserver keys.openpgp.org --recv-keys KEY_ID
```

### "gpg: WARNING: This key is not certified with a trusted signature!"

This is normal if you haven't explicitly trusted the key. Verify the fingerprint manually:
```bash
gpg --fingerprint KEY_ID
```

Compare with published fingerprint on website/README.

### "gpg: signing failed: Inappropriate ioctl for device"

Set GPG TTY:
```bash
export GPG_TTY=$(tty)
```

### Signature Verification Fails

1. Re-download the file (may be corrupted)
2. Verify you have the correct public key
3. Check that signature file matches binary file
4. Report to maintainers if issue persists

## Security Considerations

### Attack Scenarios

1. **Compromised Signing Key**
   - Immediately revoke key
   - Publish revocation certificate
   - Generate new key
   - Re-sign all recent releases

2. **Compromised GitHub Account**
   - Enable 2FA on all maintainer accounts
   - Use hardware security keys
   - Require signed commits

3. **Compromised Build Environment**
   - Use reproducible builds
   - Multiple maintainers verify builds
   - Sign in isolated environment

### Defense in Depth

Don't rely on signatures alone:
- ✅ GPG signatures
- ✅ SHA256 checksums
- ✅ Reproducible builds
- ✅ Multiple independent verifications
- ✅ Transparency logs

## Verification Checklist

Before installing a release:

- [ ] Downloaded from official GitHub releases page
- [ ] GPG signature verified (`gpg --verify`)
- [ ] Key fingerprint matches published fingerprint
- [ ] SHA256 checksum matches
- [ ] Release tag is signed
- [ ] Multiple maintainers signed (for critical releases)
- [ ] Release notes reviewed
- [ ] No reports of compromise

## Reporting Security Issues

If you discover a compromised signature or suspect a security issue:

1. **DO NOT** publicize immediately
2. Email: security@drachma.org
3. Include: What you found, steps to reproduce
4. Wait for response before disclosure

## Resources

- [GPG Documentation](https://gnupg.org/documentation/)
- [Signing Git Tags](https://git-scm.com/book/en/v2/Git-Tools-Signing-Your-Work)
- [OpenPGP Best Practices](https://riseup.net/en/security/message-security/openpgp/best-practices)
- [Keyserver Network](https://keys.openpgp.org/)

## Example Full Verification Script

```bash
#!/bin/bash
# verify-release.sh - Complete release verification

VERSION="v0.1.0"
RELEASE_URL="https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/${VERSION}"
FILENAME="drachma-${VERSION}-linux-x86_64.tar.gz"

# Download files
wget "${RELEASE_URL}/${FILENAME}"
wget "${RELEASE_URL}/${FILENAME}.asc"
wget "${RELEASE_URL}/${FILENAME}.sha256"
wget "${RELEASE_URL}/parthenon-signing-key.asc"

# Import key
gpg --import parthenon-signing-key.asc

# Verify signature
echo "==> Verifying GPG signature..."
gpg --verify "${FILENAME}.asc" "${FILENAME}" || exit 1

# Verify checksum
echo "==> Verifying checksum..."
sha256sum -c "${FILENAME}.sha256" || exit 1

echo "==> ✅ Verification successful!"
echo "==> Safe to extract: tar -xzf ${FILENAME}"
```

Save as `verify-release.sh`, chmod +x, and run to verify any release.

## Questions?

For questions about signing and verification:
- GitHub Discussions: https://github.com/Tsoympet/PARTHENON-CHAIN/discussions
- Security: security@drachma.org
