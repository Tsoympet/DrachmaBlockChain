/**
 * Secure Storage Service
 * 
 * Provides encrypted storage for sensitive data using device keychain/keystore
 */

import {MMKV} from 'react-native-mmkv';
import * as Keychain from 'react-native-keychain';

export class SecureStorage {
  private storage: MMKV;
  private encryptionKey: string | null = null;

  constructor() {
    // Initialize MMKV with encryption
    this.storage = new MMKV({
      id: 'drachma-wallet-storage',
      encryptionKey: this.getEncryptionKey(),
    });
  }

  /**
   * Get or create encryption key from device keychain
   */
  private getEncryptionKey(): string {
    // This is a placeholder - implement proper key derivation in production
    return 'drachma-default-encryption-key';
  }

  /**
   * Store item securely
   */
  async setItem(key: string, value: string): Promise<void> {
    try {
      // For highly sensitive data (like private keys), use Keychain
      if (key.includes('private') || key.includes('mnemonic')) {
        await Keychain.setGenericPassword(key, value, {
          service: key,
          accessible: Keychain.ACCESSIBLE.WHEN_UNLOCKED_THIS_DEVICE_ONLY,
        });
      } else {
        // For other data, use encrypted MMKV
        this.storage.set(key, value);
      }
    } catch (error) {
      console.error('Error storing item:', error);
      throw new Error('Failed to store item securely');
    }
  }

  /**
   * Retrieve item
   */
  async getItem(key: string): Promise<string | null> {
    try {
      // Check Keychain first for sensitive data
      if (key.includes('private') || key.includes('mnemonic')) {
        const credentials = await Keychain.getGenericPassword({service: key});
        return credentials ? credentials.password : null;
      } else {
        // Get from MMKV
        return this.storage.getString(key) || null;
      }
    } catch (error) {
      console.error('Error retrieving item:', error);
      return null;
    }
  }

  /**
   * Remove item
   */
  async removeItem(key: string): Promise<void> {
    try {
      if (key.includes('private') || key.includes('mnemonic')) {
        await Keychain.resetGenericPassword({service: key});
      } else {
        this.storage.delete(key);
      }
    } catch (error) {
      console.error('Error removing item:', error);
      throw new Error('Failed to remove item');
    }
  }

  /**
   * Clear all storage (use with caution!)
   */
  async clear(): Promise<void> {
    this.storage.clearAll();
    await Keychain.resetGenericPassword();
  }

  /**
   * Check if key exists
   */
  contains(key: string): boolean {
    return this.storage.contains(key);
  }
}

export default SecureStorage;
