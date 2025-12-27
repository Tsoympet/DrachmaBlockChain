const rpcInput = document.getElementById('rpc');
const networkEl = document.getElementById('network');
const tipEl = document.getElementById('tip');
const blockInput = document.getElementById('block-id');
const blockJsonEl = document.getElementById('block-json');
const txInput = document.getElementById('tx-id');
const txJsonEl = document.getElementById('tx-json');
const addressInput = document.getElementById('address');
const addressSummaryEl = document.getElementById('address-summary');
const mempoolEl = document.getElementById('mempool');
const blockTxList = document.getElementById('block-txs');

async function rpcCall(method, params = []) {
  const rpcUrl = rpcInput.value.trim();
  const response = await fetch(rpcUrl, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ jsonrpc: '2.0', id: 'explorer', method, params })
  });
  const obj = await response.json();
  if (obj.error) throw new Error(obj.error.message || 'RPC error');
  return obj.result;
}

async function refreshTip() {
  try {
    tipEl.textContent = 'Loading…';
    const [height, hash, netInfo] = await Promise.all([
      rpcCall('getblockcount'),
      rpcCall('getbestblockhash'),
      rpcCall('getnetworkinfo').catch(() => ({ network: 'unknown' }))
    ]);
    const network = typeof netInfo === 'object' ? netInfo.network || 'unknown' : netInfo;
    tipEl.textContent = `Height: ${height} / Hash: ${hash}`;
    networkEl.textContent = `Network: ${network}`;
  } catch (err) {
    tipEl.textContent = `Error: ${err.message}`;
    networkEl.textContent = '';
  }
}

async function lookupBlock() {
  const id = blockInput.value.trim();
  if (!id) return;
  blockJsonEl.textContent = 'Loading…';
  blockTxList.innerHTML = '';
  try {
    const blockHash = isNaN(Number(id)) ? id : await rpcCall('getblockhash', [Number(id)]);
    const block = await rpcCall('getblock', [blockHash, 2]);
    blockJsonEl.textContent = JSON.stringify(block, null, 2);

    if (Array.isArray(block.tx)) {
      const txRows = block.tx.slice(0, 10).map((tx) => {
        const totalOut = tx.vout?.reduce((sum, v) => sum + (v.value || 0), 0) || 0;
        return `<li><code>${tx.txid}</code> — ${tx.vout?.length || 0} outputs, total ${totalOut} DRM</li>`;
      });
      blockTxList.innerHTML = txRows.join('');
      if (block.tx.length > 10) {
        blockTxList.innerHTML += `<li>…and ${block.tx.length - 10} more</li>`;
      }
    }
  } catch (err) {
    blockJsonEl.textContent = `Error: ${err.message}`;
  }
}

async function lookupTx() {
  const txid = txInput.value.trim();
  if (!txid) return;
  txJsonEl.textContent = 'Loading…';
  try {
    const tx = await rpcCall('gettransaction', [txid]);
    txJsonEl.textContent = JSON.stringify(tx, null, 2);
  } catch (err) {
    txJsonEl.textContent = `Error: ${err.message}`;
  }
}

async function lookupAddress() {
  const address = addressInput.value.trim();
  if (!address) return;
  addressSummaryEl.textContent = 'Loading…';
  try {
    const [balance, utxos] = await Promise.all([
      rpcCall('getbalance', [address]),
      rpcCall('listunspent', [address]).catch(() => [])
    ]);
    const lines = [`Balance: ${balance}`, `Spendable outputs: ${utxos.length}`];
    if (utxos.length) {
      lines.push('UTXOs:');
      utxos.slice(0, 10).forEach((u) => lines.push(`${u.txid}:${u.vout} amount=${u.amount}`));
      if (utxos.length > 10) lines.push('…');
    }
    addressSummaryEl.textContent = lines.join('\n');
  } catch (err) {
    addressSummaryEl.textContent = `Error: ${err.message}`;
  }
}

async function refreshMempool() {
  mempoolEl.textContent = 'Loading…';
  try {
    const txids = await rpcCall('getrawmempool');
    mempoolEl.textContent = txids.join('\n');
  } catch (err) {
    mempoolEl.textContent = `Error: ${err.message}`;
  }
}

function wireEvents() {
  document.getElementById('refresh').addEventListener('click', refreshTip);
  document.getElementById('lookup').addEventListener('click', lookupBlock);
  document.getElementById('tx-lookup').addEventListener('click', lookupTx);
  document.getElementById('address-lookup').addEventListener('click', lookupAddress);
  document.getElementById('mempool-refresh').addEventListener('click', refreshMempool);
}

wireEvents();
refreshTip();
