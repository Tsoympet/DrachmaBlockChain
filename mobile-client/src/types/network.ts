/**
 * Network-related TypeScript types
 */

export type NetworkType = 'mainnet' | 'testnet' | 'regtest';

export interface NetworkInfo {
  type: NetworkType;
  name: string;
  rpcUrl: string;
  rpcPort: number;
  explorer?: string;
  chainId?: number;
}

export interface NetworkState {
  type: NetworkType;
  isConnected: boolean;
  blockHeight: number;
  peerCount: number;
  isSyncing: boolean;
  syncProgress: number;
  rpcUrl: string;
  latency?: number;
  lastBlockTime?: number;
}

export interface PeerInfo {
  id: string;
  address: string;
  version: string;
  services: string;
  syncedBlocks: number;
  latency: number;
}
