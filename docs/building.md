# Building DRACHMA

This guide outlines how to build the DRACHMA reference implementation on common platforms. The project uses CMake and a C++17 toolchain.

## Prerequisites

- CMake >= 3.18
- C++17 compiler (GCC >= 10 or Clang >= 11)
- OpenSSL (for cryptography and RPC)
- Boost (filesystem, program_options, asio)
- SQLite or LevelDB equivalents as introduced in future releases
- Python 3 (for scripts/tests)
- CUDA Toolkit or OpenCL SDK for GPU miners (optional)

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential cmake libssl-dev libboost-all-dev python3
# GPU miners (optional)
sudo apt install -y ocl-icd-opencl-dev
```

## Configure & Build

```bash
git clone https://github.com/Tsoympet/BlockChainDrachma.git
cd BlockChainDrachma
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Artifacts are produced under `build/` following the repository layout (e.g., `build/layer1-core/`, `build/layer3-app/`).

### Debug Builds

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
ctest --test-dir build-debug --output-on-failure
```

### Deterministic/CI Builds

- Use a pinned toolchain container or the provided `Dockerfile` for reproducibility.
- Avoid system-wide library overrides; prefer explicit paths via `-D` flags when necessary.

## Running

- **Node (testnet):** `./build/layer1-core/drachmad --network testnet --datadir ~/.drachma`
- **Services:** run the Layer 2 daemon after the node to expose RPC and mempool services.
- **Desktop app:** `./build/layer3-app/drachma-wallet --connect 127.0.0.1:9333`
- **Miners:** see `docs/mining-guide.md`.

## Troubleshooting

- Ensure submodules are initialized if introduced later: `git submodule update --init --recursive`.
- Delete the build directory when switching compilers or major dependencies.
- For GPU miners, verify drivers and toolkit versions; set `CUDA_HOME` or `OpenCL` paths when needed.

## Platform Notes

- **Linux:** primary development target; CI runs here first.
- **macOS:** may require `brew install cmake openssl boost`. Set `OPENSSL_ROOT_DIR` if autodetection fails.
- **Windows:** use MSVC 2019+ or clang-cl with Visual Studio generator. Prefer vcpkg/Conan for dependencies.

## Next Steps

- Read `docs/architecture.md` for the layered model.
- Follow `CONTRIBUTING.md` before submitting patches.
- Use `SECURITY.md` to report vulnerabilities privately.
