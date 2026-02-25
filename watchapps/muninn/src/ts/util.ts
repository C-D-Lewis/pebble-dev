import {
  MAX_SYNC_ITEMS,
  SECONDS_PER_DAY,
  STATUS_CHARGED,
  MIN_CHARGE_AMOUNT,
  STATUS_EMPTY
} from './constants';
import { HistoryItem } from './types';

/**
 * Test scenario:
 *   32 samples
 *   2 charge events
 *   Steady 2% drain otherwise
 *   4 samples per day
 */
export const generateTestData = (): HistoryItem[] => {
  const now = Math.floor(Date.now() / 1000);
  const change = 2;
  const items: HistoryItem[] = [];
  for (let i = 0; i < MAX_SYNC_ITEMS; i++) {
    const timestamp = now - (MAX_SYNC_ITEMS - i) * (SECONDS_PER_DAY / 4);
    const result = (i === 32 - 8 || i === 32 - 16) ? STATUS_CHARGED : (change * 4);
    const chargeDiff = result === STATUS_CHARGED ? MIN_CHARGE_AMOUNT + 10 : change;
    const rate = result === STATUS_CHARGED ? STATUS_EMPTY : chargeDiff * 4;
    const chargePerc = 100 - (i * change); // Just a dummy percentage
    const timeDiff = SECONDS_PER_DAY / 4;
    items.push({ timestamp, result, chargeDiff, rate, chargePerc, timeDiff });
  }
  return items;
};
