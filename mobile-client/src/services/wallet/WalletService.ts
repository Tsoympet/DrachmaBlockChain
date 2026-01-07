/**
 * Wallet Service for PARTHENON CHAIN Mobile Wallet
 * 
 * Handles key management, transaction signing, and wallet operations.
 * Uses BIP39 for mnemonic generation and HD wallet derivation.
 */

import * as bip39 from 'bip39';
import {secp256k1} from '@noble/secp256k1';
import {Buffer} from 'buffer';
import {SecureStorage} from '../storage/SecureStorage';

export interface WalletAccount {
  address: string;
  publicKey: string;
  privateKey?: string; // Only stored encrypted
  derivationPath: string;
  index: number;
}

export interface WalletData {
  mnemonic?: string; // Encrypted
  accounts: WalletAccount[];
  currentAccountIndex: number;
}

export class WalletService {
  private static WALLET_KEY = 'drachma_wallet_data';
  private storage: SecureStorage;

  constructor() {
    this.storage = new SecureStorage();
  }

  /**
   * Generate a new wallet with BIP39 mnemonic
   */
  async generateWallet(passphrase = ''): Promise<string> {
    // Generate 24-word mnemonic (256 bits entropy)
    const mnemonic = bip39.generateMnemonic(256);
    
    // Derive seed from mnemonic
    const seed = await bip39.mnemonicToSeed(mnemonic, passphrase);
    
    // Create first account (m/44'/0'/0'/0/0)
    const account = await this.deriveAccount(seed, 0);
    
    // Store wallet data securely
    const walletData: WalletData = {
      mnemonic,
      accounts: [account],
      currentAccountIndex: 0,
    };
    
    await this.storage.setItem(WalletService.WALLET_KEY, JSON.stringify(walletData));
    
    return mnemonic;
  }

  /**
   * Restore wallet from mnemonic
   */
  async restoreWallet(mnemonic: string, passphrase = ''): Promise<void> {
    // Validate mnemonic
    if (!bip39.validateMnemonic(mnemonic)) {
      throw new Error('Invalid mnemonic phrase');
    }
    
    // Derive seed
    const seed = await bip39.mnemonicToSeed(mnemonic, passphrase);
    
    // Create first account
    const account = await this.deriveAccount(seed, 0);
    
    // Store wallet data
    const walletData: WalletData = {
      mnemonic,
      accounts: [account],
      currentAccountIndex: 0,
    };
    
    await this.storage.setItem(WalletService.WALLET_KEY, JSON.stringify(walletData));
  }

  /**
   * Derive account from seed using BIP44 path
   */
  private async deriveAccount(seed: Buffer, index: number): Promise<WalletAccount> {
    // For now, use simple derivation (in production, use proper BIP32/BIP44)
    const path = `m/44'/0'/0'/0/${index}`;
    
    // Derive private key (simplified - use proper BIP32 in production)
    const privateKey = secp256k1.utils.hashToPrivateKey(
      Buffer.concat([seed.slice(0, 32), Buffer.from([index])])
    );
    
    // Derive public key
    const publicKey = secp256k1.getPublicKey(privateKey, true);
    
    // Generate address (Schnorr-based, 32-byte pubkey)
    const address = this.generateAddress(publicKey);
    
    return {
      address,
      publicKey: Buffer.from(publicKey).toString('hex'),
      privateKey: Buffer.from(privateKey).toString('hex'),
      derivationPath: path,
      index,
    };
  }

  /**
   * Generate Drachma address from public key
   */
  private generateAddress(publicKey: Uint8Array): string {
    // Drachma uses Schnorr signatures with 32-byte pubkeys
    // This is a simplified version - use proper address encoding in production
    const hash = secp256k1.utils.sha256(publicKey);
    return 'drm' + Buffer.from(hash).toString('hex').slice(0, 40);
  }

  /**
   * Get current wallet data
   */
  async getWallet(): Promise<WalletData | null> {
    const data = await this.storage.getItem(WalletService.WALLET_KEY);
    return data ? JSON.parse(data) : null;
  }

  /**
   * Get current account
   */
  async getCurrentAccount(): Promise<WalletAccount | null> {
    const wallet = await this.getWallet();
    if (!wallet || wallet.accounts.length === 0) return null;
    return wallet.accounts[wallet.currentAccountIndex];
  }

  /**
   * Create new account
   */
  async createAccount(): Promise<WalletAccount> {
    const wallet = await this.getWallet();
    if (!wallet || !wallet.mnemonic) {
      throw new Error('No wallet found');
    }
    
    const seed = await bip39.mnemonicToSeed(wallet.mnemonic);
    const newIndex = wallet.accounts.length;
    const account = await this.deriveAccount(seed, newIndex);
    
    wallet.accounts.push(account);
    await this.storage.setItem(WalletService.WALLET_KEY, JSON.stringify(wallet));
    
    return account;
  }

  /**
   * Switch to different account
   */
  async switchAccount(index: number): Promise<void> {
    const wallet = await this.getWallet();
    if (!wallet || index >= wallet.accounts.length) {
      throw new Error('Invalid account index');
    }
    
    wallet.currentAccountIndex = index;
    await this.storage.setItem(WalletService.WALLET_KEY, JSON.stringify(wallet));
  }

  /**
   * Sign transaction (simplified - implement proper signing in production)
   */
  async signTransaction(txData: any): Promise<string> {
    const account = await this.getCurrentAccount();
    if (!account || !account.privateKey) {
      throw new Error('No account available for signing');
    }
    
    const privateKey = Buffer.from(account.privateKey, 'hex');
    const txHash = secp256k1.utils.sha256(Buffer.from(JSON.stringify(txData)));
    
    // Sign with Schnorr
    const signature = await secp256k1.schnorr.sign(txHash, privateKey);
    
    return Buffer.from(signature).toString('hex');
  }

  /**
   * Delete wallet (use with caution!)
   */
  async deleteWallet(): Promise<void> {
    await this.storage.removeItem(WalletService.WALLET_KEY);
  }

  /**
   * Check if wallet exists
   */
  async hasWallet(): Promise<boolean> {
    return !!(await this.getWallet());
  }
}

export default WalletService;
