import {
  MAX_SYNC_ITEMS,
  STATUS_EMPTY,
  TEST_SAMPLE_DATA,
} from './constants';
import {
  calculateDischargeRate,
  calculateLastWeekRate,
  calculateNumCharges,
  calculateMeanTimeBetweenCharges,
} from './stats';
import { HistoryItem } from './types';
import { ensureUploadId } from './upload';
import { buildHistoryKey, generateTestData } from './util';

/**
 * Save the history of samples.
 */
const saveHistory = (history: HistoryItem[]) => {
  const start = Date.now();
  localStorage.setItem(buildHistoryKey(), JSON.stringify(history));
  console.log(`saveHistory: ${history.length} items in ${Date.now() - start}ms`);
};

/**
 * Load the history of samples.
 */
export const loadHistory = (): HistoryItem[] => {
  if (TEST_SAMPLE_DATA) return generateTestData();

  try {
    const start = Date.now();
    const arr = JSON.parse(localStorage.getItem(buildHistoryKey()) || '[]');
    // console.log(JSON.stringify(arr));
    console.log(`loadHistory: ${arr.length} items in ${Date.now() - start}ms`);
    return arr;
  } catch (e) {
    console.error('Failed to load history');
    console.error(e);

    // Can't load JSON, user can't fix. Reset it.
    return [];
  }
};

/**
 * Send the watch the last seen timestamp so updates can be incremental.
 */
export const handleGetSyncInfo = async () => {
  const history = loadHistory();
  // console.log(JSON.stringify(history));

  let lastTs = STATUS_EMPTY;
  if (history.length > 0) {
    lastTs = history[0].timestamp;
  }

  const uploadId = await ensureUploadId();

  const res = {
    SYNC_TIMESTAMP: lastTs,
    SYNC_COUNT: history.length,
    STAT_TOTAL_DAYS: Math.floor(history.length / 4),
    STAT_ALL_TIME_RATE: calculateDischargeRate(history),
    STAT_LAST_WEEK_RATE: calculateLastWeekRate(history),
    STAT_NUM_CHARGES: calculateNumCharges(history),
    STAT_MTBC: calculateMeanTimeBetweenCharges(history),
    UPLOAD_ID: uploadId,
  };
  console.log(JSON.stringify(res));
  await PebbleTS.sendAppMessage(res);
};

/**
 * Importing data should be as robust as possible.
 *
 * TODO: How do we handle adding new fields?
 */
export const handleSync = async (dict: Record<string, any>) => {
  // Ignore the watch sending data if we're using the generated data
  if (TEST_SAMPLE_DATA) return;

  let history = loadHistory();
  const timestamp = dict.SAMPLE_TIMESTAMP;

  // Manual loop replaces .find() which coreapp doesn't like
  for (let i = 0; i < history.length; i++) {
    if (history[i].timestamp === timestamp) {
      console.log(`Duplicate: ${timestamp}`);
      return;
    }
  }

  // Construct local sample
  const sample: HistoryItem = {
    timestamp: dict.SAMPLE_TIMESTAMP,
    chargePerc: dict.SAMPLE_CHARGE_PERC,
    timeDiff: dict.SAMPLE_TIME_DIFF,
    chargeDiff: dict.SAMPLE_CHARGE_DIFF,
    rate: dict.SAMPLE_RATE,
    result: dict.SAMPLE_RESULT,
  };
  history.push(sample);

  // Sort to match watch order and limit length
  history = history
    .sort((a, b) => a.timestamp > b.timestamp ? -1 : 1)
    .slice(0, MAX_SYNC_ITEMS);

  saveHistory(history);
  console.log(`Saved: ${JSON.stringify(sample)} (length: ${history.length})`);
};

/**
 * Delete history for this watch.
 */
export const deleteWatchHistory = () => {
  localStorage.removeItem(buildHistoryKey());
};
