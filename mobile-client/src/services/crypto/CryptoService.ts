/**
 * Cryptographic Operations Service
 *
 * Provides encryption, decryption, hashing, and key derivation utilities.
 */

import * as secp from '@noble/secp256k1';
import {sha256 as sha256Hash} from '@noble/hashes/sha2';
import {Buffer} from 'buffer';
import * as Random from 'expo-random';

export class CryptoService {
  /**
   * Generate a random private key
   */
  static generatePrivateKey(): Uint8Array {
    return secp.utils.randomPrivateKey();
  }

  /**
   * Derive public key from private key
   */
  static getPublicKey(privateKey: Uint8Array, compressed = true): Uint8Array {
    return secp.getPublicKey(privateKey, compressed);
  }

  /**
   * Hash data using SHA256
   */
  static sha256(data: Uint8Array | string): Uint8Array {
    const buffer = typeof data === 'string' ? Buffer.from(data, 'utf8') : data;
    return sha256Hash(buffer);
  }

  /**
   * Sign message with Schnorr signature
   */
  static async schnorrSign(message: Uint8Array, privateKey: Uint8Array): Promise<Uint8Array> {
    return await secp.schnorr.sign(message, privateKey);
  }

  /**
   * Verify Schnorr signature
   */
  static async schnorrVerify(
    signature: Uint8Array,
    message: Uint8Array,
    publicKey: Uint8Array,
  ): Promise<boolean> {
    return await secp.schnorr.verify(signature, message, publicKey);
  }

  /**
   * Sign message with ECDSA signature
   */
  static async ecdsaSign(message: Uint8Array, privateKey: Uint8Array): Promise<Uint8Array> {
    return await secp.sign(message, privateKey);
  }

  /**
   * Verify ECDSA signature
   */
  static async ecdsaVerify(
    signature: Uint8Array,
    message: Uint8Array,
    publicKey: Uint8Array,
  ): Promise<boolean> {
    return secp.verify(signature, message, publicKey);
  }

  /**
   * Convert hex string to Uint8Array
   */
  static hexToBytes(hex: string): Uint8Array {
    return Buffer.from(hex, 'hex');
  }

  /**
   * Convert Uint8Array to hex string
   */
  static bytesToHex(bytes: Uint8Array): string {
    return Buffer.from(bytes).toString('hex');
  }

  /**
   * Generate secure random bytes
   */
  static randomBytes(length: number): Uint8Array {
    return Random.getRandomBytes(length);
  }
}

export default CryptoService;
