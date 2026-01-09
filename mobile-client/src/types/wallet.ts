/**
 * Wallet-related TypeScript types
 */

export interface WalletAccount {
  address: string;
  publicKey: string;
  label?: string;
  balance: number;
  derivationPath?: string;
}

export interface WalletData {
  mnemonic?: string;
  accounts: WalletAccount[];
  currentAccountIndex: number;
  isLocked: boolean;
}

export interface WalletState {
  isInitialized: boolean;
  isLocked: boolean;
  currentAddress: string | null;
  accounts: WalletAccount[];
  balance: string;
  address: string | null;
  isLoading: boolean;
  error: string | null;
}

export interface AssetBalance {
  assetId: string;
  symbol: string;
  name: string;
  balance: number;
  decimals: number;
  usdValue?: number;
}
