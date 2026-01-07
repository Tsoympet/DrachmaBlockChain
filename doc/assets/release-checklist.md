# Release Checklist

Use this checklist when cutting a new tag (e.g., `v0.1.0-rc`). Copy into the GitHub Release body or PR description as needed.

## Preparation
- [ ] Decide scope and confirm CHANGELOG.md is updated.
- [ ] Bump versions in code/configs if required; tag commit with `vX.Y.Z[-rc|-testnet]`.
- [ ] Ensure build environment is reproducible (pinned toolchains, clean build directory).
- [ ] Review and update documentation for the release.

## Reproducible Build
- [ ] Build using reproducible build script: `./scripts/reproducible-build.sh`
- [ ] Verify build completes without errors.
- [ ] Compare checksums with independent build (if multiple maintainers available).
- [ ] Document build environment (compiler versions, dependencies).

## Build and Packaging
- [ ] Build Release binaries for Linux, Windows, and macOS (see `../getting-started/building.md`).
- [ ] Package installers/bundles (tar.gz, AppImage, zip/exe, dmg).
- [ ] Generate SHA256 checksums for all binaries: `sha256sum * > SHA256SUMS.txt`.
- [ ] Generate SBOM (Software Bill of Materials): `syft packages dir:. -o json > sbom.json`.

## Signing
- [ ] Sign release tag: `git tag -s vX.Y.Z -m "Release vX.Y.Z"`
- [ ] Sign all binary artifacts with GPG: `gpg --armor --detach-sign <file>`
- [ ] Sign checksums file: `gpg --clearsign SHA256SUMS.txt`
- [ ] Export public key: `gpg --armor --export > signing-key.asc`
- [ ] Verify all signatures locally before publishing.

**Required secrets for CI signing:**
- `GPG_PRIVATE_KEY` - ASCII-armored private key
- `GPG_PASSPHRASE` - Key passphrase (if applicable)

## Testing and Verification
- [ ] Verify hashes on a second machine/maintainer build.
- [ ] Compare deterministic outputs when possible.
- [ ] Smoke-test node, wallet, and miners on testnet.
- [ ] Confirm wallet connects to the freshly built node.
- [ ] Re-run unit/integration tests if enabled.
- [ ] Verify signatures: `gpg --verify <file>.asc <file>`
- [ ] Test installation on clean system.

## Infrastructure Readiness (Pre-Mainnet)
- [ ] Deploy and test 3+ seed nodes (see `doc/operators/seed-nodes.md`).
- [ ] Configure monitoring (Prometheus/Grafana) for all infrastructure.
- [ ] Set up alerting for critical issues.
- [ ] Verify block explorer works for both testnet and mainnet.
- [ ] Run extended testnet validation (7+ days): `python3 scripts/extended-testnet-validation.py --duration 7`.
- [ ] Document all infrastructure deployment procedures.

## Publication
- [ ] Create a GitHub Release for the tag with:
  - [ ] Summary of changes (use `CHANGELOG.md`).
  - [ ] Upload binaries/installers.
  - [ ] Upload `SHA256SUMS.txt` and `SHA256SUMS.txt.asc`.
  - [ ] Upload GPG signatures (`.asc` files).
  - [ ] Upload SBOM (`sbom.json`).
  - [ ] Upload public signing key (`signing-key.asc`).
  - [ ] Include verification instructions.
  - [ ] Mark as pre-release for `-rc`/`-testnet` tags.
- [ ] Update doc/website links if download URLs changed.
- [ ] Publish container images to GitHub Packages (GHCR):
  - [ ] Choose **Container registry (ghcr.io)** when prompted to publish the first package.
  - [ ] Tag images as `ghcr.io/<org>/drachmad:<tag>` and `ghcr.io/<org>/drachma-wallet:<tag>` (matching the Git tag).
  - [ ] Push with `docker login ghcr.io` + `docker push`.
  - [ ] Confirm the packages appear under the **Packages** tab.

## Security
- [ ] Scan binaries for vulnerabilities.
- [ ] Review dependencies for known CVEs.
- [ ] Ensure no test keys or credentials in release.
- [ ] Verify all communication uses TLS/secure channels.

## Documentation
- [ ] Update README.md with new version references.
- [ ] Update installation guides with current version.
- [ ] Review and update API documentation.
- [ ] Add release notes to documentation site.
- [ ] Update reproducible builds documentation.

## Announcement
- [ ] Post release notes and download links to Matrix/Discord/Reddit/X.
- [ ] Share signatures and checksums; remind users to verify before running.
- [ ] Update website with new release information.
- [ ] Publish announcement blog post (if applicable).
- [ ] Update status page with release information.

## Post-Release
- [ ] Monitor GitHub issues for release-related problems.
- [ ] Track feedback and issues for quick patch releases.
- [ ] Update project status document.
- [ ] Begin planning next release cycle.
- [ ] Archive old pre-release builds if storage is limited.

## Mainnet-Specific Checklist (for mainnet launches only)
- [ ] External security audit completed and approved.
- [ ] All critical audit findings addressed.
- [ ] Genesis block finalized and verified.
- [ ] RPC layer production-hardened.
- [ ] 3+ seed nodes operational and monitored.
- [ ] Block explorer fully functional for mainnet.
- [ ] Extended stability testing completed (7+ days minimum).
- [ ] Incident response plan documented and tested.
- [ ] 24/7 monitoring configured with alerts.
- [ ] Backup and recovery procedures tested.
- [ ] Legal and compliance review completed (if applicable).
- [ ] Community coordination and miner outreach completed.

## Verification Instructions for Users

Include these instructions in the release notes:

```bash
# Download release
wget https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/vX.Y.Z/drachma-vX.Y.Z-linux-x86_64.tar.gz
wget https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/vX.Y.Z/drachma-vX.Y.Z-linux-x86_64.tar.gz.asc
wget https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/vX.Y.Z/signing-key.asc

# Import signing key
gpg --import signing-key.asc

# Verify signature
gpg --verify drachma-vX.Y.Z-linux-x86_64.tar.gz.asc drachma-vX.Y.Z-linux-x86_64.tar.gz

# Verify checksum
sha256sum -c drachma-vX.Y.Z-linux-x86_64.tar.gz.sha256

# Extract only if both verifications pass
tar -xzf drachma-vX.Y.Z-linux-x86_64.tar.gz
```

## Emergency Procedures

If issues are discovered after release:

1. **Critical Security Issue**
   - Immediately revoke release
   - Publish security advisory
   - Release patched version
   - Notify all known users

2. **Non-Critical Bug**
   - Document in known issues
   - Plan patch release
   - Update release notes

3. **Compromised Signing Key**
   - Revoke GPG key immediately
   - Publish revocation certificate
   - Re-sign all recent releases with new key
   - Announce key rotation

## Release Metrics to Track

- [ ] Download counts by platform
- [ ] Verification rate (how many users verify signatures)
- [ ] Issue reports within first 48 hours
- [ ] Node deployment rate on mainnet
- [ ] Community feedback sentiment

## Notes

- Always test on a clean system before declaring release ready
- Never skip signature verification steps
- Keep private keys secure and offline when not in use
- Coordinate with other maintainers for multi-signature releases
- Document any deviations from this checklist
