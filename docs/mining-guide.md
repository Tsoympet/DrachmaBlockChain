# DRACHMA Mining Guide

This guide explains how to run the reference CPU and GPU miners, tune performance, and choose hardware for DRACHMA's PoW.

## Miner Binaries

- **CPU Miner:** `miners/drachma-miner-cpu`
- **GPU Miner:** `miners/drachma-miner-gpu` (CUDA or OpenCL depending on build)

Both miners communicate with Layer 2 services via RPC to fetch block templates and submit solved headers.

## Prerequisites

- A synced DRACHMA node and services stack with RPC enabled.
- RPC credentials or access tokens configured (see Layer 2 service docs).
- For GPU mining: compatible drivers and CUDA/OpenCL runtime installed.

## Starting the CPU Miner

```bash
./miners/drachma-miner-cpu \
  --rpc-url http://127.0.0.1:9332 \
  --rpc-user <user> --rpc-pass <pass> \
  --threads <num>
```

- `--threads`: Number of CPU worker threads. Start with `nproc - 1` to leave one core for system tasks.
- The miner periodically requests new templates and automatically handles chain tips and difficulty adjustments.

## Starting the GPU Miner

CUDA example:
```bash
./miners/drachma-miner-gpu \
  --rpc-url http://127.0.0.1:9332 \
  --rpc-user <user> --rpc-pass <pass> \
  --backend cuda --devices 0,1 \
  --intensity 22
```

OpenCL example:
```bash
./miners/drachma-miner-gpu \
  --rpc-url http://127.0.0.1:9332 \
  --rpc-user <user> --rpc-pass <pass> \
  --backend opencl --platform 0 --devices 0 \
  --work-size 256
```

Key flags:
- `--backend`: `cuda` or `opencl`.
- `--devices`: Comma-separated device indices.
- `--platform`: OpenCL platform index (for AMD/Intel stacks).
- `--intensity` / `--work-size`: Tune kernel occupancy and batch size.

## Performance Tuning

- **CPU:**
  - Use hugepages and NUMA pinning on multi-socket systems.
  - Set `--threads` to match physical cores; hyper-threading yields diminishing returns.
  - Pin miner threads with `taskset` or `numactl` to reduce cross-node latency.
- **GPU:**
  - Match `--intensity`/`--work-size` to available VRAM and bus bandwidth.
  - Keep GPUs cool; hash rate drops when thermal throttling triggers.
  - Use recent drivers; outdated OpenCL ICDs often reduce stability.
- **Node/Network:**
  - Run miners close to the node (same LAN) to minimize RPC latency.
  - Ensure the services daemon has sufficient peers for timely template updates.

## Hardware Recommendations

- **Entry / Development:** 4–8 core CPU, integrated or low-end GPU; suitable for testing.
- **Prosumer:** 12–24 core CPU, modern NVIDIA RTX (30/40 series) or AMD RDNA2/3 GPUs with >= 8 GB VRAM.
- **Enterprise:** Multi-GPU rigs with high-efficiency PSUs, adequate cooling, and stable power delivery. Consider ECC memory on hosts for reliability.

## Monitoring and Stability

- Enable verbose logs during tuning: `--log-level debug`.
- Watch acceptance rate; frequent rejects indicate stale work or unstable overclocks.
- Restart miners after driver updates to reload kernels.

## Troubleshooting

- **RPC auth failures:** Verify credentials and ensure RPC is bound to the expected interface.
- **Low hash rate:** Reduce intensity, update drivers, or test another backend (CUDA vs OpenCL).
- **Stales or rejects:** Improve connectivity to peers, lower overclocks, and ensure system clocks are synchronized (NTP).
- **Build/runtime errors:** Rebuild miners with matching driver/toolkit versions; see `docs/building.md` for GPU build notes.
