#!/usr/bin/env bash
# Deploy PARTHENON CHAIN seed node
# This script sets up and runs a production seed node

set -euo pipefail

# Configuration
NODE_NAME="${1:-seed-node-1}"
NETWORK="${2:-testnet}"
DATA_DIR="${3:-/var/lib/drachma-${NODE_NAME}}"
CONFIG_DIR="/etc/drachma"
LOG_DIR="/var/log/drachma"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

info() {
    echo -e "${GREEN}[INFO]${NC} $*"
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $*"
}

error() {
    echo -e "${RED}[ERROR]${NC} $*"
}

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    error "This script must be run as root (use sudo)"
    exit 1
fi

info "Deploying PARTHENON CHAIN seed node: ${NODE_NAME}"
info "Network: ${NETWORK}"
info "Data directory: ${DATA_DIR}"

# Create necessary directories
info "Creating directories..."
mkdir -p "${DATA_DIR}"
mkdir -p "${CONFIG_DIR}"
mkdir -p "${LOG_DIR}"

# Create drachma user if it doesn't exist
if ! id -u drachma &>/dev/null; then
    info "Creating drachma user..."
    useradd -r -s /bin/false -d "${DATA_DIR}" drachma
fi

# Set ownership
chown -R drachma:drachma "${DATA_DIR}"
chown -R drachma:drachma "${LOG_DIR}"

# Create configuration file
info "Creating node configuration..."
cat > "${CONFIG_DIR}/${NODE_NAME}.conf" <<EOF
# PARTHENON CHAIN Seed Node Configuration
# Node: ${NODE_NAME}
# Network: ${NETWORK}

# Network settings
network=${NETWORK}
listen=1
maxconnections=125

# RPC settings
# NOTE: Credentials are randomly generated with 256+ bits of entropy
# Config file is set to mode 600 (owner read/write only)
server=1
rpcuser=rpcuser_$(head -c 16 /dev/urandom | base64 | tr -dc 'a-zA-Z0-9')
rpcpassword=$(head -c 32 /dev/urandom | base64 | tr -dc 'a-zA-Z0-9')
rpcallowip=127.0.0.1
rpcport=$([[ "${NETWORK}" == "mainnet" ]] && echo "8332" || echo "18332")

# Network ports
port=$([[ "${NETWORK}" == "mainnet" ]] && echo "9333" || echo "19333")

# Logging
debug=net
debug=mempool
logips=1

# Performance
dbcache=512
maxmempool=300

# Seed node specific
discover=1
upnp=0
EOF

chown root:root "${CONFIG_DIR}/${NODE_NAME}.conf"
chmod 600 "${CONFIG_DIR}/${NODE_NAME}.conf"

# Create systemd service
info "Creating systemd service..."
cat > "/etc/systemd/system/drachma-${NODE_NAME}.service" <<EOF
[Unit]
Description=PARTHENON CHAIN Seed Node (${NODE_NAME})
After=network.target

[Service]
Type=simple
User=drachma
Group=drachma
WorkingDirectory=${DATA_DIR}
ExecStart=/usr/local/bin/drachmad \\
    --datadir=${DATA_DIR} \\
    --conf=${CONFIG_DIR}/${NODE_NAME}.conf
Restart=always
RestartSec=10
StandardOutput=append:${LOG_DIR}/${NODE_NAME}.log
StandardError=append:${LOG_DIR}/${NODE_NAME}-error.log

# Hardening
PrivateTmp=true
NoNewPrivileges=true
ProtectSystem=full
ProtectHome=true

[Install]
WantedBy=multi-user.target
EOF

# Reload systemd
info "Reloading systemd..."
systemctl daemon-reload

# Install logrotate configuration
info "Configuring log rotation..."
cat > "/etc/logrotate.d/drachma-${NODE_NAME}" <<EOF
${LOG_DIR}/${NODE_NAME}.log ${LOG_DIR}/${NODE_NAME}-error.log {
    daily
    rotate 14
    compress
    delaycompress
    notifempty
    missingok
    postrotate
        systemctl reload drachma-${NODE_NAME} > /dev/null 2>&1 || true
    endscript
}
EOF

info "Seed node deployment complete!"
echo ""
info "Next steps:"
echo "  1. Review configuration: ${CONFIG_DIR}/${NODE_NAME}.conf"
echo "  2. Start the node: systemctl start drachma-${NODE_NAME}"
echo "  3. Enable auto-start: systemctl enable drachma-${NODE_NAME}"
echo "  4. Check status: systemctl status drachma-${NODE_NAME}"
echo "  5. View logs: journalctl -u drachma-${NODE_NAME} -f"
echo ""
warn "Remember to configure firewall rules:"
if [ "${NETWORK}" == "mainnet" ]; then
    echo "  sudo ufw allow 9333/tcp  # P2P"
    echo "  sudo ufw allow 8332/tcp from 127.0.0.1  # RPC (localhost only)"
else
    echo "  sudo ufw allow 19333/tcp  # P2P testnet"
    echo "  sudo ufw allow 18332/tcp from 127.0.0.1  # RPC testnet (localhost only)"
fi
