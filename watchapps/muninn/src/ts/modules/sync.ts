import { MAX_SYNC_ITEMS, STATUS_EMPTY, TEST_SAMPLE_DATA } from '../constants';
import {
  calculateDischargeRate,
  calculateEstimatedBatteryLife,
  calculateLastWeekRate,
  calculateMeanTimeBetweenCharges,
} from './stats';
import { HistoryItem } from '../types';
import { ensureUploadId } from './upload';
import { buildHistoryKey } from './util';
import { loadHistory, saveHistory } from './history';
import { savePluginStats } from './plugin';

/**
 * Send the watch the last seen timestamp so updates can be incremental.
 */
export const handleGetSyncInfo = async (dict: Record<string, any>) => {
  const history = loadHistory();
  // console.log(JSON.stringify(history));

  let lastTs = STATUS_EMPTY;
  if (history.length > 0) {
    lastTs = history[0].timestamp;
  }

  const uploadId = await ensureUploadId();

  const now = new Date().getTime();
  const batteryLife = calculateEstimatedBatteryLife(history, now);
  const allTimeRate = calculateDischargeRate(history);
  const lastWeekRate = calculateLastWeekRate(history, now);
  const meanTimeBetweenCharges = calculateMeanTimeBetweenCharges(history);

  const res = {
    SYNC_TIMESTAMP: lastTs,
    SYNC_COUNT: history.length,
    STAT_TOTAL_DAYS: Math.floor(history.length / 4),
    STAT_ALL_TIME_RATE: String(allTimeRate),
    STAT_LAST_WEEK_RATE: String(lastWeekRate),
    STAT_BATTERY_LIFE: batteryLife,
    STAT_MTBC: meanTimeBetweenCharges,
    UPLOAD_ID: uploadId,
  };
  console.log(JSON.stringify(res));
  await PebbleTS.sendAppMessage(res);

  // Also update the Pebble plugin
  savePluginStats({
    daysRemaining: dict.DAYS_REMAINING, // This is sent as part of watchapp response to the ready event
    batteryLife,
    lastWeekRate,
    updatedAt: now,
  });
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
