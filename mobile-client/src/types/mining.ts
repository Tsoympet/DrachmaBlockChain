/**
 * Mining-related TypeScript types
 */

export interface MiningConfig {
  enabled: boolean;
  poolUrl?: string;
  workerName?: string;
  minerAddress?: string;
  threads?: number;

  // Mobile-specific
  maxBatteryDrain?: number;
  enableOnBattery?: boolean;
  enableOnCharging?: boolean;
  minBatteryLevel?: number;
  maxTemperature?: number;
}

export interface MiningStats {
  isActive: boolean;
  hashRate: number;
  sharesFound: number;
  sharesAccepted: number;
  sharesRejected: number;
  uptime: number;
  difficulty?: number;
  lastShareTime?: number;

  // Mobile-specific
  temperature?: number;
  batteryLevel?: number;
  isCharging?: boolean;
}

export interface MiningJob {
  jobId: string;
  target: string;
  difficulty: number;
  height: number;
  prevHash: string;
  merkleRoot: string;
  timestamp: number;
  nonce?: number;
}

export interface MiningPoolInfo {
  url: string;
  name: string;
  fee: number;
  minPayout: number;
  paymentInterval: number;
  status: 'active' | 'inactive' | 'maintenance';
}
