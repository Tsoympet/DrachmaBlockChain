# BlockChainDrachma Miners

This directory contains CPU and GPU mining implementations for the BlockChainDrachma blockchain.

## Available Miners

### 1. CPU Miner (`drachma_cpu_miner`)
Multi-threaded CPU miner with AVX2 optimizations (when available).

**Usage:**
```bash
# Benchmark mode
./drachma_cpu_miner --benchmark --threads 4 --benchmark-seconds 10

# Pool mining
./drachma_cpu_miner --stratum-url stratum+tcp://pool.example.com:3333 \
                     --stratum-user worker1 --stratum-pass x

# Config file mode
./drachma_cpu_miner --config miner_config.json

# Standalone mining (for testing)
./drachma_cpu_miner <version> <prevhash> <merkleroot> <time> <bits> [threads]
```

### 2. CUDA Miner (`drachma_cuda_miner`)
GPU miner for NVIDIA CUDA-compatible devices.

**Usage:**
```bash
# Benchmark mode
./drachma_cuda_miner --benchmark --devices 0,1 --intensity 256

# Pool mining
./drachma_cuda_miner --stratum-url stratum+tcp://pool.example.com:3333 \
                      --stratum-user worker1 --stratum-pass x \
                      --devices 0

# Config file mode
./drachma_cuda_miner --config miner_config.json
```

### 3. OpenCL Miner (`drachma_opencl_miner`)
GPU miner for OpenCL-compatible devices (AMD, Intel, etc.).

**Usage:**
```bash
# Benchmark mode
./drachma_opencl_miner --benchmark --devices 0 --intensity 256

# Pool mining
./drachma_opencl_miner --stratum-url stratum+tcp://pool.example.com:3333 \
                        --stratum-user worker1 --stratum-pass x

# Config file mode
./drachma_opencl_miner --config miner_config.json
```

## Common Command-Line Options

All three miners support the following options:

- `--benchmark` - Run benchmark mode (no pool connection)
- `--benchmark-seconds N` - Duration of benchmark in seconds (default: 10)
- `--threads N` - Number of CPU threads (CPU miner only)
- `--devices N,M,...` - GPU device indices to use (GPU miners only, comma-separated)
- `--intensity N` - Mining intensity (affects batch size)
- `--stratum-url URL` - Pool URL (e.g., `stratum+tcp://pool.example.com:3333`)
- `--stratum-user USER` - Pool username/worker name
- `--stratum-pass PASS` - Pool password
- `--worker NAME` - Worker identification name
- `--allow-remote` - Allow connections to remote pools (security flag)
- `--min-target-bits BITS` - Minimum difficulty target (hex)
- `--config FILE` - Load configuration from JSON file
- `--stratum-v2` - Use Stratum V2 protocol (if supported by pool)
- `--rpc-auth-token TOKEN` - RPC authentication token

## Configuration File Format

Example `miner_config.json`:

```json
{
  "stratum_url": "stratum+tcp://pool.example.com:3333",
  "user": "worker1",
  "pass": "x",
  "worker": "my_miner",
  "threads": 4,
  "devices": [0, 1],
  "allow_remote": false,
  "min_target_bits": 0,
  "benchmark_seconds": 10,
  "intensity": 256,
  "benchmark": false,
  "stratum_v2": false,
  "rpc_auth_token": ""
}
```

## Security Features

### Remote Connection Protection
By default, all miners reject connections to remote Stratum pools (anything other than `127.0.0.1` or `localhost`). This prevents accidental mining for unknown third parties. To enable remote pool connections, use the `--allow-remote` flag.

### Malware Detection
The `StratumPool` class includes basic malware detection that scans for common illicit mining processes on Linux systems.

## Architecture Integration

### Consensus Integration
All miners use:
- `BlockHeader` from `layer1-core/block/block.h`
- `CheckProofOfWork` from `layer1-core/pow/difficulty.h` for validation
- `consensus::Params` from `layer1-core/consensus/params.h` for network parameters

### Stratum Protocol
All miners implement:
- Stratum V1 protocol with full `mining.notify`, `mining.set_difficulty`, and `mining.set_extranonce` support
- Automatic reconnection with exponential backoff (max 10 attempts)
- Job queue management to handle multiple pending jobs
- Stale share detection (jobs older than 30 seconds)
- Connection health monitoring

### Target Validation
Both GPU kernels (CUDA and OpenCL) implement target comparison that matches the consensus rules:
- Big-endian 256-bit integer comparison
- High-to-low word order (index 7 to 0)
- Accepts hash values where `hash <= target`

## Building

### CPU Miner
Always built by default if Boost is available.

### CUDA Miner
Built automatically if CUDA Toolkit is detected:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make drachma_cuda_miner
```

### OpenCL Miner
Requires explicit enable flag:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DDRACHMA_ENABLE_OPENCL=ON ..
make drachma_opencl_miner
```

## Performance Tips

1. **CPU Mining:**
   - Set `--threads` to match your CPU core count
   - Use `--intensity` to tune batch size for cache optimization
   - Enable AVX2 if supported (automatic on x86_64)

2. **GPU Mining:**
   - Start with `--intensity 256` and adjust based on your GPU
   - Higher intensity = larger batches = better GPU utilization
   - Use `--devices` to select specific GPUs in multi-GPU setups
   - Monitor GPU temperature and reduce intensity if overheating

3. **Pool Mining:**
   - Keep miners close to pools (low latency)
   - Use `--worker` to identify different miners
   - Monitor pool dashboard for stale shares

## Troubleshooting

### "remote stratum connections require --allow-remote"
Add `--allow-remote` flag when connecting to non-local pools.

### GPU not detected
- CUDA: Check `nvidia-smi` and ensure CUDA drivers are installed
- OpenCL: Verify OpenCL runtime is installed for your GPU

### Low hashrate
- Increase `--intensity`
- For CPU, ensure no other heavy processes are running
- For GPU, check thermals and power limits

### Build failures
- Ensure Boost development libraries are installed: `libboost-system-dev`
- For CUDA: Install CUDA Toolkit matching your driver version
- For OpenCL: Install appropriate OpenCL headers and ICD loader

## License

Same as the parent BlockChainDrachma project.
