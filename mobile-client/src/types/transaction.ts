/**
 * Transaction-related TypeScript types
 */

export type TransactionType = 'send' | 'receive' | 'transfer' | 'stake' | 'unstake';
export type TransactionStatus = 'pending' | 'confirmed' | 'failed' | 'cancelled';

export interface Transaction {
  id: string;
  txid?: string;
  type: TransactionType;
  from: string;
  to: string;
  amount: string;
  assetId?: string;
  fee?: string;
  timestamp: number;
  blockHeight?: number;
  confirmations?: number;
  status: TransactionStatus;
  memo?: string;
  data?: any;
}

export interface TransactionInput {
  txid: string;
  vout: number;
  address: string;
  amount: number;
  scriptPubKey?: string;
}

export interface TransactionOutput {
  address: string;
  amount: number;
  assetId?: string;
  scriptPubKey?: string;
}

export interface TransactionDetails extends Transaction {
  inputs: TransactionInput[];
  outputs: TransactionOutput[];
  size: number;
  vsize: number;
  weight: number;
  locktime: number;
  version: number;
  hex?: string;
}

export interface PendingTransaction {
  transaction: Transaction;
  rawTx: string;
  signedTx?: string;
  isValid: boolean;
  estimatedFee: number;
}
