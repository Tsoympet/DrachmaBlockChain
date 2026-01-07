/**
 * Network State Management
 */

import {createSlice, PayloadAction} from '@reduxjs/toolkit';

export type NetworkType = 'mainnet' | 'testnet' | 'regtest';

interface NetworkState {
  type: NetworkType;
  isConnected: boolean;
  blockHeight: number;
  peerCount: number;
  isSyncing: boolean;
  syncProgress: number;
  rpcUrl: string;
}

const initialState: NetworkState = {
  type: 'testnet',
  isConnected: false,
  blockHeight: 0,
  peerCount: 0,
  isSyncing: false,
  syncProgress: 0,
  rpcUrl: 'https://tn1.drachma.org:18332',
};

const networkSlice = createSlice({
  name: 'network',
  initialState,
  reducers: {
    setNetworkType: (state, action: PayloadAction<NetworkType>) => {
      state.type = action.payload;
    },
    setConnected: (state, action: PayloadAction<boolean>) => {
      state.isConnected = action.payload;
    },
    setBlockHeight: (state, action: PayloadAction<number>) => {
      state.blockHeight = action.payload;
    },
    setPeerCount: (state, action: PayloadAction<number>) => {
      state.peerCount = action.payload;
    },
    setSyncing: (state, action: PayloadAction<boolean>) => {
      state.isSyncing = action.payload;
    },
    setSyncProgress: (state, action: PayloadAction<number>) => {
      state.syncProgress = action.payload;
    },
    setRpcUrl: (state, action: PayloadAction<string>) => {
      state.rpcUrl = action.payload;
    },
  },
});

export const {
  setNetworkType,
  setConnected,
  setBlockHeight,
  setPeerCount,
  setSyncing,
  setSyncProgress,
  setRpcUrl,
} = networkSlice.actions;

export default networkSlice.reducer;
