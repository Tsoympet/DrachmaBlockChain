/**
 * Redux Persist configuration
 */

import AsyncStorage from '@react-native-async-storage/async-storage';
import {PersistConfig} from 'redux-persist';

export const persistConfig: PersistConfig<any> = {
  key: 'drachma-wallet',
  storage: AsyncStorage,
  whitelist: ['network', 'wallet', 'mining'],
};
