# Reproducible Builds

## Overview

PARTHENON CHAIN implements reproducible (deterministic) builds to ensure that anyone can verify that release binaries match the source code. This is critical for security and trust in a monetary blockchain.

## What are Reproducible Builds?

Reproducible builds ensure that compiling the same source code with the same build environment produces bit-for-bit identical binaries. This allows independent verification that:
- Release binaries contain no malicious code
- The published source code matches the distributed executables
- No backdoors were inserted during the build process

## Requirements

To build PARTHENON CHAIN reproducibly, you need:

### System Requirements
- Linux (Ubuntu 20.04+ or Debian 11+ recommended)
- Docker (optional, for containerized builds)
- At least 4GB RAM
- 10GB free disk space

### Build Tools
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev \
    libleveldb-dev
```

## Building Reproducibly

### Method 1: Using the Reproducible Build Script (Recommended)

```bash
# Clone the repository
git clone https://github.com/Tsoympet/PARTHENON-CHAIN.git
cd PARTHENON-CHAIN

# Checkout the specific release tag
git checkout v0.1.0

# Run reproducible build
./scripts/reproducible-build.sh
```

This script:
1. Sets deterministic build flags
2. Disables machine-specific optimizations
3. Uses portable compiler flags
4. Generates SHA256 checksums
5. Creates a distributable archive

Output will be in `dist/drachma-v0.1.0-linux-x86_64.tar.gz`

### Method 2: Manual Reproducible Build

```bash
# Set environment for reproducibility
export SOURCE_DATE_EPOCH=1
export TZ=UTC
export LC_ALL=C
export CFLAGS="-O2 -march=x86-64 -fno-asynchronous-unwind-tables"
export CXXFLAGS="-O2 -march=x86-64 -fno-asynchronous-unwind-tables"
export LDFLAGS="-Wl,--build-id=none"

# Configure
cmake -S . -B build-reproducible \
    -DCMAKE_BUILD_TYPE=Release \
    -DDRACHMA_BUILD_TESTS=OFF \
    -DDRACHMA_BUILD_GUI=OFF \
    -DCMAKE_C_FLAGS="${CFLAGS}" \
    -DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
    -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}"

# Build
cmake --build build-reproducible --parallel

# Generate checksums
find build-reproducible -type f -executable -name "drachma*" -exec sha256sum {} \; > checksums.txt
```

### Method 3: Docker-based Build (Most Reproducible)

```bash
# Build using the Dockerfile
docker build -t drachma-build -f Dockerfile.reproducible .

# Extract binaries
docker create --name drachma-temp drachma-build
docker cp drachma-temp:/opt/drachma/dist ./dist
docker rm drachma-temp
```

## Verifying a Release

To verify that a release binary matches the source:

```bash
# Download the release
wget https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/v0.1.0/drachma-v0.1.0-linux-x86_64.tar.gz
wget https://github.com/Tsoympet/PARTHENON-CHAIN/releases/download/v0.1.0/drachma-v0.1.0-linux-x86_64.tar.gz.sha256

# Verify checksum
sha256sum -c drachma-v0.1.0-linux-x86_64.tar.gz.sha256

# Build from source at the same tag
git clone https://github.com/Tsoympet/PARTHENON-CHAIN.git
cd PARTHENON-CHAIN
git checkout v0.1.0
./scripts/reproducible-build.sh

# Compare checksums
diff <(sha256sum dist/drachma-v0.1.0-linux-x86_64.tar.gz) \
     <(cat drachma-v0.1.0-linux-x86_64.tar.gz.sha256)
```

If the checksums match, the binary is verified!

## Key Principles

### 1. Fixed Timestamps
We use `SOURCE_DATE_EPOCH=1` to ensure all timestamps in binaries are deterministic.

### 2. Portable Architecture
Instead of `-march=native`, we use `-march=x86-64` to ensure binaries work on any x86_64 CPU.

### 3. No Build IDs
We disable build IDs (`-Wl,--build-id=none`) as they can vary between builds.

### 4. Stripped Binaries
All release binaries are stripped to remove debug symbols and ensure consistency.

### 5. Fixed Locale
We use `LC_ALL=C` and `TZ=UTC` to ensure consistent sorting and timestamps.

## Platform Support

Currently, reproducible builds are fully supported on:
- **Linux x86_64** - Fully tested and verified

Partial support (verification pending):
- **macOS** - Requires additional work for deterministic code signing
- **Windows** - Cross-compilation from Linux recommended

## Common Issues

### Issue: Different Checksums on Different Machines

**Cause:** Different compiler versions or system libraries.

**Solution:** Use Docker-based builds or ensure identical build environments.

### Issue: Binaries Don't Run on Older CPUs

**Cause:** Using `-march=native` instead of `-march=x86-64`.

**Solution:** Use the reproducible build script, which sets the correct flags.

### Issue: Build Takes Too Long

**Cause:** Debug symbols or unoptimized builds.

**Solution:** Ensure you're using Release build type and stripped binaries.

## Contributing

If you can verify a release on your machine:

1. Follow the verification steps above
2. Post your results in the GitHub Discussion
3. Include your system info: `uname -a`, `gcc --version`, `cmake --version`

Multiple independent verifications increase trust in our releases!

## Advanced: Gitian Builds

For maximum reproducibility, we plan to implement Gitian builds in the future. Gitian uses virtualization to create identical build environments.

This will be documented in a future update once implemented.

## Resources

- [Reproducible Builds Project](https://reproducible-builds.org/)
- [Bitcoin's Gitian Builds](https://github.com/bitcoin-core/docs/blob/master/gitian-building.md)
- [Debian Reproducible Builds](https://wiki.debian.org/ReproducibleBuilds)

## Questions?

For questions about reproducible builds:
- Open an issue: https://github.com/Tsoympet/PARTHENON-CHAIN/issues
- Discussion: https://github.com/Tsoympet/PARTHENON-CHAIN/discussions
- Security concerns: security@drachma.org
