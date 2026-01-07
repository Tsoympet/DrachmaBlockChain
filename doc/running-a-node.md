# Running a PARTHENON CHAIN Node

Complete guide for running a full PARTHENON CHAIN node as a validator or to support the network.

## Table of Contents

- [Introduction](#introduction)
- [Prerequisites](#prerequisites)
- [Initial Setup](#initial-setup)
- [Configuration](#configuration)
- [Starting the Node](#starting-the-node)
- [Syncing the Blockchain](#syncing-the-blockchain)
- [Monitoring Your Node](#monitoring-your-node)
- [Maintenance](#maintenance)
- [Advanced Configuration](#advanced-configuration)

## Introduction

Running a full node helps secure the PARTHENON CHAIN network by:
- Validating and relaying transactions
- Storing a complete copy of the blockchain
- Participating in network consensus
- Providing data to light clients

## Prerequisites

### Hardware

**Minimum:**
- CPU: 2 cores (x86_64)
- RAM: 4 GB
- Disk: 100 GB free space
- Network: 10 Mbps up/down

**Recommended:**
- CPU: 4+ cores
- RAM: 8+ GB
- Disk: 500 GB SSD
- Network: 100 Mbps up/down, unlimited data

### Software

- Linux (Ubuntu 20.04+), Windows 10+, or macOS 11+
- PARTHENON CHAIN binaries (see [Installation Guide](./install.md))

## Initial Setup

### 1. Install Binaries

Follow the [Installation Guide](./install.md) to download and verify binaries.

### 2. Create Data Directory

The node stores blockchain data in a data directory:

```bash
# Linux/macOS default
mkdir -p ~/.drachma

# Or specify custom location
mkdir -p /mnt/blockchain/drachma
```

### 3. Configure Firewall

Open the P2P port (default: 9333) for incoming connections:

**Linux (ufw):**
```bash
sudo ufw allow 9333/tcp
sudo ufw enable
```

**Linux (iptables):**
```bash
sudo iptables -A INPUT -p tcp --dport 9333 -j ACCEPT
sudo iptables-save > /etc/iptables/rules.v4
```

**Windows Firewall:**
```powershell
New-NetFirewallRule -DisplayName "PARTHENON P2P" -Direction Inbound -Protocol TCP -LocalPort 9333 -Action Allow
```

**macOS:**
- System Preferences > Security & Privacy > Firewall > Firewall Options
- Add drachmad and allow incoming connections

## Configuration

Create a configuration file for persistent settings:

**Location:**
- Linux/macOS: `~/.drachma/drachma.conf`
- Windows: `%APPDATA%\Drachma\drachma.conf`

### Basic Configuration

```ini
# Network
network=mainnet

# P2P Settings
port=9333
listen=1
maxconnections=125

# RPC Settings
rpcuser=your_username_here
rpcpassword=your_strong_password_here
rpcport=8332
rpcallowip=127.0.0.1

# Performance
dbcache=4096
```

### Security Recommendations

1. **Use strong RPC credentials:**
   ```ini
   rpcuser=node_$(openssl rand -hex 8)
   rpcpassword=$(openssl rand -base64 32)
   ```

2. **Restrict RPC access:**
   ```ini
   # Only allow localhost
   rpcallowip=127.0.0.1
   ```

3. **Keep software updated:**
   - Subscribe to release notifications
   - Apply security updates promptly

## Starting the Node

### First Start

```bash
# Start daemon
drachmad

# Or with custom config
drachmad --conf=/path/to/drachma.conf

# Or with inline options
drachmad --datadir=/mnt/blockchain/drachma --rpcuser=myuser --rpcpassword=mypass
```

The node will:
1. Initialize the database
2. Load the configuration
3. Connect to network peers
4. Begin downloading blocks

### Running as a Service

For production deployments, run as a system service:

#### Linux (systemd)

Create `/etc/systemd/system/drachmad.service`:

```ini
[Unit]
Description=PARTHENON CHAIN Daemon
After=network.target

[Service]
Type=simple
User=drachma
Group=drachma
ExecStart=/usr/local/bin/drachmad -daemon=0
Restart=on-failure
RestartSec=10

# Hardening
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=full
ProtectHome=true

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl enable drachmad
sudo systemctl start drachmad
sudo systemctl status drachmad
```

View logs:
```bash
sudo journalctl -u drachmad -f
```

#### Windows (NSSM)

1. Download NSSM: https://nssm.cc/download
2. Install service:
   ```cmd
   nssm install drachmad "C:\Program Files\PARTHENON-CHAIN\drachmad.exe"
   nssm start drachmad
   ```

#### macOS (launchd)

Create `~/Library/LaunchAgents/org.parthenon.drachmad.plist`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
"http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>org.parthenon.drachmad</string>
    <key>ProgramArguments</key>
    <array>
        <string>/usr/local/bin/drachmad</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
</dict>
</plist>
```

Load service:
```bash
launchctl load ~/Library/LaunchAgents/org.parthenon.drachmad.plist
```

## Syncing the Blockchain

### Initial Block Download (IBD)

The first sync downloads the entire blockchain. This can take hours to days depending on:
- Network speed
- Disk I/O performance
- CPU speed
- Current blockchain size

### Monitor Sync Progress

```bash
# Check current block height
drachma-cli getblockcount

# Get detailed sync status
drachma-cli getinfo

# Watch sync in real-time
watch -n 10 drachma-cli getblockcount
```

### Optimization Tips

1. **Use SSD storage:**
   - Significantly faster than HDD
   - Reduce sync time by 50-75%

2. **Increase database cache:**
   ```ini
   dbcache=8192  # 8 GB (use ~50% of RAM)
   ```

3. **Increase connections:**
   ```ini
   maxconnections=200
   ```

4. **Use checkpoint data (if available):**
   - Download blockchain bootstrap
   - Verify checksums before using

### Verify Sync Status

Node is synced when:
```bash
# Your block count matches network height
drachma-cli getblockcount
# Compare with block explorer

# No longer showing "syncing" in getinfo
drachma-cli getinfo | grep syncing
```

## Monitoring Your Node

### Key Metrics to Track

#### 1. Block Height
```bash
drachma-cli getblockcount
```

#### 2. Peer Connections
```bash
drachma-cli getpeerinfo | grep addr
```

#### 3. Network Traffic
```bash
drachma-cli getnettotals
```

#### 4. Memory Usage
```bash
# Linux
ps aux | grep drachmad

# Or use drachma-cli
drachma-cli getmemoryinfo
```

#### 5. Disk Usage
```bash
du -sh ~/.drachma
```

### Logging

Enable detailed logging:

```ini
# In drachma.conf
debug=net
debug=rpc
logips=1
```

View logs:
```bash
# Linux/macOS
tail -f ~/.drachma/debug.log

# Windows
type %APPDATA%\Drachma\debug.log
```

### Health Checks

Create a monitoring script:

```bash
#!/bin/bash
# node-health.sh

BLOCKCOUNT=$(drachma-cli getblockcount 2>/dev/null)
PEERCOUNT=$(drachma-cli getpeerinfo 2>/dev/null | grep -c "addr")

echo "Block Height: $BLOCKCOUNT"
echo "Peer Count: $PEERCOUNT"

if [ $PEERCOUNT -lt 8 ]; then
    echo "WARNING: Low peer count"
fi

# Add to crontab for regular checks
# */5 * * * * /path/to/node-health.sh >> /var/log/drachma-health.log
```

## Maintenance

### Regular Tasks

#### 1. Update Software

```bash
# Stop node
drachma-cli stop

# Backup wallet (if applicable)
cp ~/.drachma/wallet.dat ~/wallet-backup-$(date +%Y%m%d).dat

# Download and verify new release
# (see Installation Guide)

# Replace binaries
sudo install -m 755 drachmad /usr/local/bin/

# Restart
drachmad
```

#### 2. Backup Data

Critical files to backup:
- `wallet.dat` - Your wallet (if you have one)
- `drachma.conf` - Configuration
- `peers.dat` - Known peers list

```bash
# Automated backup script
#!/bin/bash
BACKUP_DIR=~/drachma-backups/$(date +%Y%m%d)
mkdir -p $BACKUP_DIR

cp ~/.drachma/wallet.dat $BACKUP_DIR/
cp ~/.drachma/drachma.conf $BACKUP_DIR/
cp ~/.drachma/peers.dat $BACKUP_DIR/

# Encrypt backup (optional)
tar -czf - $BACKUP_DIR | openssl enc -aes-256-cbc -e > $BACKUP_DIR.tar.gz.enc
```

#### 3. Monitor Disk Space

```bash
# Check usage
df -h ~/.drachma

# If running low, consider:
# - Moving to larger drive
# - Pruning old blocks (if supported)
```

### Troubleshooting

#### Node won't sync

1. Check internet connection
2. Verify P2P port is open
3. Check peer count: `drachma-cli getpeerinfo`
4. Try adding seed nodes manually

#### High CPU usage

- Normal during initial sync
- If persistent after sync, check for:
  - Excessive logging
  - Resource-intensive RPC calls
  - Malware/mining malware

#### Database corruption

```bash
# Stop daemon
drachma-cli stop

# Rebuild index (if supported)
drachmad --reindex

# Last resort: resync from genesis
rm -rf ~/.drachma/blocks ~/.drachma/chainstate
drachmad
```

## Advanced Configuration

### Networking

```ini
# Bind to specific interface
bind=192.168.1.100

# Use proxy
proxy=127.0.0.1:9050

# Connect to specific nodes only
connect=node1.example.com:9333
connect=node2.example.com:9333
```

### Performance Tuning

```ini
# Maximum signature cache size (MB)
maxsigcachesize=100

# Number of script verification threads
par=4

# Maximum mempool size (MB)
maxmempool=300
```

### Security

```ini
# Require encrypted RPC connections (if supported)
rpcsslcertificatechainfile=/path/to/cert.pem
rpcsslprivatekeyfile=/path/to/key.pem

# Whitelist specific peers
whitelist=192.168.1.0/24

# Ban misbehaving nodes
bantime=86400
```

## Best Practices

1. **Keep backups:**
   - Regular automated backups
   - Store offsite
   - Test restoration

2. **Monitor proactively:**
   - Set up alerts for low disk space
   - Track peer count
   - Monitor block height

3. **Secure your node:**
   - Use firewall rules
   - Keep RPC password strong
   - Run as non-root user

4. **Stay informed:**
   - Subscribe to release announcements
   - Join community channels
   - Read security advisories

## Resources

- [Installation Guide](./install.md)
- [Mining Guide](./mining-guide.md)
- [RPC API Reference](./rpc-reference.md)
- [Security Best Practices](../security/security-overview.md)

## Support

- GitHub Issues: https://github.com/Tsoympet/PARTHENON-CHAIN/issues
- Discussions: https://github.com/Tsoympet/PARTHENON-CHAIN/discussions

## License

PARTHENON CHAIN is released under the MIT License. See LICENSE file for details.
