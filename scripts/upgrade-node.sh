#!/bin/bash
set -euo pipefail

INSTALL_DIR=${INSTALL_DIR:-/usr/local/bin}
SERVICE_NAME=""
NEW_BINARY=""
RESTART=0
EXPECTED_SHA=""

usage() {
  cat <<USAGE
Usage: $0 -b <path-to-new-binary> [-i install_dir] [-s systemd_service] [--restart] [--sha256 <digest>]

Safely upgrades a running node by backing up the previous binary, copying the
new build into place, and optionally restarting a systemd-managed service.
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help) usage; exit 0 ;;
    -b) NEW_BINARY="$2"; shift 2 ;;
    -i) INSTALL_DIR="$2"; shift 2 ;;
    -s) SERVICE_NAME="$2"; shift 2 ;;
    --restart) RESTART=1; shift ;;
    --sha256) EXPECTED_SHA="$2"; shift 2 ;;
    --) shift; break ;;
    *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
  esac
done

if [[ -z "$NEW_BINARY" ]]; then
  echo "New binary path is required" >&2
  usage
  exit 1
fi

if [[ ! -x "$NEW_BINARY" ]]; then
  echo "Provided binary is not executable: $NEW_BINARY" >&2
  exit 1
fi

TARGET_BIN="${INSTALL_DIR}/drachma_node"
mkdir -p "$INSTALL_DIR"

if [[ -n "$EXPECTED_SHA" ]]; then
  ACTUAL_SHA=$(sha256sum "$NEW_BINARY" | awk '{print $1}')
  if [[ "$ACTUAL_SHA" != "$EXPECTED_SHA" ]]; then
    echo "SHA256 mismatch: expected $EXPECTED_SHA got $ACTUAL_SHA" >&2
    exit 1
  fi
fi

TIMESTAMP=$(date +%Y%m%d%H%M%S)
if [[ -f "$TARGET_BIN" ]]; then
  cp "$TARGET_BIN" "${TARGET_BIN}.${TIMESTAMP}.bak"
  echo "Backed up existing binary to ${TARGET_BIN}.${TIMESTAMP}.bak"
fi

if [[ -n "$SERVICE_NAME" ]] && command -v systemctl >/dev/null; then
  systemctl stop "$SERVICE_NAME" || true
fi

install -m 0755 "$NEW_BINARY" "$TARGET_BIN"

echo "Installed new binary to $TARGET_BIN"

if [[ "$RESTART" -eq 1 ]]; then
  if [[ -n "$SERVICE_NAME" ]] && command -v systemctl >/dev/null; then
    systemctl start "$SERVICE_NAME"
    systemctl status --no-pager "$SERVICE_NAME" || true
  else
    echo "No service specified; use scripts/start-node.sh to launch manually." >&2
  fi
else
  echo "Restart skipped; start the daemon with scripts/start-node.sh or your service manager." >&2
fi
