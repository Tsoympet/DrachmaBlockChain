# DRACHMA Deployment Guide

This guide covers hardened procedures for running DRACHMA nodes on the public
testnet and preparing for mainnet launch.

## Prerequisites
- 64-bit Linux host with recent kernel, `curl`, `jq`, and `docker`/`docker-compose` (for containerized setups).
- Open firewall for the chosen P2P port (default 19335 testnet / 9333 mainnet).
- Dedicated user account to run the daemon; avoid running as root.

## Building from source
1. Clone the repository and run `scripts/build.sh`.
2. Verify the resulting binary hash matches the published release notes.
3. Record the commit hash used for reproducibility and attestation.

## Running a node
- Quick start: `scripts/start-node.sh -n testnet` will create a minimal config in
  `~/.drachma-testnet` and launch the daemon using the built binary.
- For production, copy `mainnet/config.sample.conf` into your datadir, adjust
  RPC credentials, and run `scripts/start-node.sh -n mainnet -c /path/to/drachma.conf`.
- Confirm connectivity with `scripts/sync-check.sh --network testnet` (or
  `mainnet`) which validates checkpoint height and block freshness.

## Docker-based multi-node testnet
- `docker-compose up -d` launches two seed nodes, three regular peers, a faucet,
  Prometheus, and Grafana. The compose file exposes RPC on 18332/18333/18334 and
  metrics on 9311 for the first seed.
- Tail logs with `docker-compose logs -f drachma-seed-a` and inspect metrics via
  Grafana at http://localhost:3000 (default password `drachma`).

## Security hardening
- Restrict RPC access to localhost or VPN ranges; always set strong RPC
  credentials and consider TLS termination via a reverse proxy for public
  faucets.
- Keep `checkpoints.json` up-to-date to resist long-range forks and configure
  `maxuploadtarget` and `banscore` as in `mainnet/config.sample.conf`.
- Enable systemd service isolation with `ProtectSystem=strict`, `NoNewPrivileges`
  and `PrivateTmp=yes`.

## Upgrades
- Use `scripts/upgrade-node.sh -b /path/to/new/drachma_node --sha256 <digest>
  --restart -s drachma.service` to atomically install new binaries and restart a
  managed service.
- Always back up configs and the `wallet.dat` (if applicable) before upgrading;
  the script automatically preserves the previous binary with a timestamp.

## Monitoring and health
- Prometheus and Grafana dashboards live in `testnet/monitoring/`; update scrape
  targets if your hostnames differ from the default compose stack.
- `scripts/sync-check.sh` alarms when the tip age exceeds a threshold or the node
  lags published checkpoints—use in cron or CI.

## Backups and recovery
- Periodically back up the data directory and `drachma.conf`. For wallet nodes,
  export encrypted backups via the layer3 app’s backup menu or copy `wallet.dat`
  while the wallet is locked.
- Validate backups by restoring onto an isolated host and verifying `getbalance`
  and a few historical transactions via RPC.
