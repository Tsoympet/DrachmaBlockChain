#!/usr/bin/env bash
# Reproducible build script for PARTHENON CHAIN
# This script ensures deterministic builds across different machines
#
# Usage: ./scripts/reproducible-build.sh [--version VERSION]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build-reproducible"

# Default version or from git tag
VERSION="${1:-$(git describe --tags --always --dirty 2>/dev/null || echo "v0.1.0-dev")}"

echo "==> Building PARTHENON CHAIN ${VERSION} (reproducible)"

# Clean previous build
if [ -d "${BUILD_DIR}" ]; then
    echo "==> Cleaning previous build directory"
    rm -rf "${BUILD_DIR}"
fi

# Set reproducible build environment variables
export SOURCE_DATE_EPOCH=1
export TZ=UTC
export LC_ALL=C

# Disable optimizations that may vary by machine
# Use -march=x86-64 instead of -march=native for portability
export CFLAGS="-O2 -march=x86-64 -fno-asynchronous-unwind-tables"
export CXXFLAGS="-O2 -march=x86-64 -fno-asynchronous-unwind-tables"
export LDFLAGS="-Wl,--build-id=none"

echo "==> Configuring with reproducible settings"
cmake -S "${REPO_ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DDRACHMA_BUILD_TESTS=OFF \
    -DDRACHMA_BUILD_GUI=OFF \
    -DDRACHMA_COVERAGE=OFF \
    -DCMAKE_C_FLAGS="${CFLAGS}" \
    -DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
    -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}"

echo "==> Building binaries"
cmake --build "${BUILD_DIR}" --parallel "$(nproc 2>/dev/null || echo 4)"

echo "==> Collecting binaries"
DIST_DIR="${REPO_ROOT}/dist/${VERSION}"
mkdir -p "${DIST_DIR}"

# Find and copy core binaries
BINARIES=(drachmad drachma-cli drachma_cpu_miner)
for binary in "${BINARIES[@]}"; do
    binary_path=$(find "${BUILD_DIR}" -type f -name "${binary}" -executable 2>/dev/null | head -1)
    if [ -n "${binary_path}" ]; then
        cp "${binary_path}" "${DIST_DIR}/"
        echo "   Copied: ${binary}"
    else
        echo "   Warning: ${binary} not found"
    fi
done

# Strip binaries deterministically
if command -v strip &> /dev/null; then
    echo "==> Stripping binaries"
    for binary in "${DIST_DIR}"/*; do
        if [ -f "${binary}" ] && [ -x "${binary}" ]; then
            strip --strip-all "${binary}" 2>/dev/null || true
        fi
    done
fi

echo "==> Generating checksums"
cd "${DIST_DIR}"
sha256sum * > SHA256SUMS.txt
cd "${REPO_ROOT}"

echo "==> Creating archive"
ARCHIVE_NAME="drachma-${VERSION}-linux-x86_64.tar.gz"
tar -czf "dist/${ARCHIVE_NAME}" -C "${DIST_DIR}" .

echo "==> Build complete"
echo "    Binaries: ${DIST_DIR}"
echo "    Archive:  dist/${ARCHIVE_NAME}"
echo ""
echo "==> Checksums:"
cat "${DIST_DIR}/SHA256SUMS.txt"

# Generate final archive checksum
cd "${REPO_ROOT}/dist"
sha256sum "${ARCHIVE_NAME}" > "${ARCHIVE_NAME}.sha256"
echo ""
echo "Archive checksum:"
cat "${ARCHIVE_NAME}.sha256"
