# DRACHMA Mining Guide

This guide describes how to run the reference CPU and GPU miners against a DRACHMA node.

## Prerequisites

- A running Layer 1 node and Layer 2 services exposing mining RPC (default ports shown below).
- Credentials for RPC (user/password or token).
- For GPU miners: updated drivers and CUDA/OpenCL runtime.

## Configuration

1. Build the project (see `docs/building.md`).
2. Ensure your node is listening for miner RPC submissions:
   ```bash
   ./build/layer1-core/drachmad --network testnet --rpcuser=user --rpcpassword=pass --rpcbind=127.0.0.1 --rpcport=8332
   ```
3. Start Layer 2 services (if separated) to expose work templates on port `9333` (example):
   ```bash
   ./build/layer2-services/drachma-service --connect 127.0.0.1:8333 --work-port 9333
   ```

## CPU Miner

```bash
./build/miners/cpu-miner/drachma-cpuminer \
  --rpc http://127.0.0.1:8332 \
  --user user --pass pass \
  --threads 4
```

- Adjust `--threads` to match available CPU cores.
- Monitor logs for accepted/rejected shares and block submissions.

## GPU Miner (CUDA)

```bash
./build/miners/gpu-miner/drachma-cuda \
  --url 127.0.0.1:9333 \
  --user user --pass pass \
  --intensity 22 --devices 0
```

- Tune `--intensity` based on GPU memory/thermal limits.
- Use `--devices` to select GPUs (comma-separated indices).

## GPU Miner (OpenCL)

```bash
./build/miners/gpu-miner/drachma-opencl \
  --url 127.0.0.1:9333 \
  --user user --pass pass \
  --platform 0 --device 0 --work-batch 256
```

- Use `--list-devices` to enumerate available platforms/devices.
- Start with modest batch sizes to avoid watchdog resets.

## Best Practices

- Keep your node and miners on stable power and connectivity.
- Use dedicated wallets for mining payouts and regularly back up keys.
- Monitor temperatures; configure throttling to avoid hardware damage.
- Prefer LAN connections between miners and the node to minimize latency.

## Troubleshooting

- **Connection refused:** confirm RPC ports, credentials, and firewalls.
- **Low hashrate:** check driver versions, intensity settings, and CPU scaling.
- **Stale shares:** reduce network latency, increase work refresh frequency, and ensure clocks are synchronized (NTP).

## Safety Notes

- Do not run untrusted miner binaries. Build from source or verify signatures.
- Avoid running miners as root; use dedicated users with minimal permissions.
- Report suspected vulnerabilities privately via `SECURITY.md`.
