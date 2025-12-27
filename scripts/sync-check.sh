#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NETWORK=${NETWORK:-testnet}
RPC_URL=${RPC_URL:-"http://127.0.0.1:18332"}
RPC_USER=${RPC_USER:-"user"}
RPC_PASS=${RPC_PASSWORD:-"pass"}
CHECKPOINT_FILE=""
MAX_AGE=${MAX_AGE:-7200}

usage() {
  cat <<USAGE
Usage: $0 [-n network] [-u rpc_url] [-U user] [-P pass] [-f checkpoint_file] [-a max_age_seconds]

Validates that the node is caught up to known checkpoints and that the best block
is fresh relative to wall-clock time.
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help) usage; exit 0 ;;
    -n) NETWORK="$2"; shift 2 ;;
    -u) RPC_URL="$2"; shift 2 ;;
    -U) RPC_USER="$2"; shift 2 ;;
    -P) RPC_PASS="$2"; shift 2 ;;
    -f) CHECKPOINT_FILE="$2"; shift 2 ;;
    -a) MAX_AGE="$2"; shift 2 ;;
    --) shift; break ;;
    *) echo "Unknown option $1" >&2; usage; exit 1 ;;
  esac
done

if [[ -z "$CHECKPOINT_FILE" ]]; then
  DEFAULT_CP="${ROOT_DIR}/${NETWORK}/checkpoints.json"
  [[ -f "$DEFAULT_CP" ]] && CHECKPOINT_FILE="$DEFAULT_CP"
fi

call_rpc() {
  local method=$1
  local params=${2:-"[]"}
  curl -s --user "${RPC_USER}:${RPC_PASS}" --data-binary "{\"jsonrpc\":\"1.0\",\"id\":\"sync\",\"method\":\"${method}\",\"params\":${params}}" -H 'content-type: text/plain;' "${RPC_URL}" | jq -r '.result'
}

if ! command -v jq >/dev/null; then
  echo "jq is required for sync-check" >&2
  exit 1
fi

HEIGHT=$(call_rpc getblockcount)
TIP_HASH=$(call_rpc getbestblockhash)
TIP_HEADER=$(call_rpc getblockheader "[\"${TIP_HASH}\", true]")
TIP_TIME=$(echo "$TIP_HEADER" | jq -r '.time // 0')
NOW=$(date +%s)

if [[ -z "$HEIGHT" || "$HEIGHT" == "null" ]]; then
  echo "Unable to query node height" >&2
  exit 1
fi

if [[ -n "$CHECKPOINT_FILE" && -f "$CHECKPOINT_FILE" ]]; then
  TARGET_HEIGHT=$(jq 'max_by(.height).height' "$CHECKPOINT_FILE")
  if [[ -n "$TARGET_HEIGHT" && "$HEIGHT" -lt "$TARGET_HEIGHT" ]]; then
    echo "Node height ${HEIGHT} is below checkpoint ${TARGET_HEIGHT} from ${CHECKPOINT_FILE}" >&2
    exit 2
  fi
fi

AGE=$((NOW - TIP_TIME))
if [[ "$TIP_TIME" -eq 0 ]]; then
  echo "Could not parse tip time from RPC" >&2
  exit 1
fi

echo "Network: ${NETWORK} | Height: ${HEIGHT} | Tip: ${TIP_HASH} | Age (s): ${AGE}"
if [[ "$AGE" -gt "$MAX_AGE" ]]; then
  echo "Tip is older than ${MAX_AGE}s; node may be stalled" >&2
  exit 3
fi

echo "Sync check passed"
