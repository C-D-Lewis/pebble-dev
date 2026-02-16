import {
  MAX_ITEMS,
  MIN_CHARGE_AMOUNT,
  SECONDS_PER_DAY,
  STATUS_CHARGED,
  STATUS_EMPTY,
  STATUS_NO_CHANGE,
  TEST_SAMPLE_DATA,
} from './constants';
import { HistoryItem } from './types';
import { generateTestData } from './util';

/** LocalStorage key - data history on a per-watch basis */
const buildHistoryKey = () => {
  const key = `history-${Pebble.getWatchToken()}`;
  // console.log(JSON.stringify({ key }));
  return key;
};

/**
 * Save the history of samples.
 */
const saveHistory = (history: HistoryItem[]) => {
  localStorage.setItem(buildHistoryKey(), JSON.stringify(history));
};

/**
 * Load the history of samples.
 */
const loadHistory = (): HistoryItem[] => {
  if (TEST_SAMPLE_DATA) return generateTestData();

  try {
    return JSON.parse(localStorage.getItem(buildHistoryKey()) || '[]');
  } catch (e) {
    console.error('Failed to load history');
    console.error(e);

    // Can't load JSON, user can't fix. Reset it.
    return [];
  }
};

/**
 * Average discharge rate using all history samples.
 */
const calculateDischargeRate = (history: HistoryItem[]): number => {
  if (history.length === 0) return STATUS_EMPTY;

  let count = 0;
  let rateSum = 0;
  history.forEach((p) => {
    const { rate, result } = p;

    if ([STATUS_EMPTY, STATUS_CHARGED].includes(result)) return;
    // No change, but time still elapsed
    if (result === STATUS_NO_CHANGE) {
      count++;
      return;
    }

    // FIXME: Why is this happening?
    if (!rate) {
      console.log(`WARN: HistoryItem had no rate: ${JSON.stringify(p)}`);
      return;
    }

    // Discharged, count it
    count++;
    rateSum += rate;
  });
  const result = count > 0 ? Math.round(rateSum / count) : STATUS_EMPTY;
  // console.log(JSON.stringify({ rateSum, count, result }));
  return result;
};

/**
 * Calculate the average discharge rate for the last week only.
 */
const calculateLastWeekRate = (history: HistoryItem[]): number => {
  // Assumes 4 samples per day, we need a whole week's worth to start
  if (history.length < 28) return STATUS_EMPTY;

  // Use a subset of items for the last week only
  const now = new Date().getTime();
  const subset = history.filter((p) => {
    const itemDate = new Date(p.timestamp * 1000).getTime();
    const diffD = (now - itemDate) / (SECONDS_PER_DAY * 1000);
    // console.log({ itemDate, diffD, now });
    return diffD <= 7;
  });

  return calculateDischargeRate(subset);
};

/**
 * Determine if a history item is a valid charge event.
 */
const isChargeEvent = (item: HistoryItem) =>
  item.result === STATUS_CHARGED && item.chargeDiff >= MIN_CHARGE_AMOUNT;

/**
 * Calculate the number of significant charge events.
 */
const calculateNumCharges = (history: HistoryItem[]): number =>
  history.filter(isChargeEvent).length;

/**
 * Calculate mean time between charge events, in days.
 */
const calculateMeanTimeBetweenCharges = (history: HistoryItem[]): number => {
  if (calculateNumCharges(history) < 2) return STATUS_EMPTY;

  // Get timestamps of charge events
  const chargeTimes = history
    .filter(isChargeEvent)
    .map(p => p.timestamp)
    .sort((a, b) => a - b);

  // Calculate time differences between consecutive charges
  const timeDiffs = [];
  for (let i = 1; i < chargeTimes.length; i++) {
    timeDiffs.push(chargeTimes[i] - chargeTimes[i - 1]);
  }

  // Calculate mean time between charges (in days)
  const meanTimeS = timeDiffs.reduce((acc, p) => acc + p, 0) / timeDiffs.length;
  const meanDays = meanTimeS / SECONDS_PER_DAY;
  // console.log({ chargeTimes, timeDiffs, meanTimeS, meanDays });
  
  return Math.round(meanDays);
};

/**
 * Send the watch the last seen timestamp so updates can be incremental.
 */
export const handleGetSyncInfo = async () => {
  const history = loadHistory();
  // console.log(JSON.stringify(history));

  let timestamp = STATUS_EMPTY;
  if (history.length > 0) {
    timestamp = history[0].timestamp;
  }

  const res = {
    SYNC_TIMESTAMP: timestamp,
    SYNC_COUNT: history.length,
    STAT_TOTAL_DAYS: Math.floor(history.length / 4),
    STAT_ALL_TIME_RATE: calculateDischargeRate(history),
    STAT_LAST_WEEK_RATE: calculateLastWeekRate(history),
    STAT_NUM_CHARGES: calculateNumCharges(history),
    STAT_MTBC: calculateMeanTimeBetweenCharges(history),
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
    .slice(0, MAX_ITEMS);

  saveHistory(history);
  console.log(`Saved: ${JSON.stringify(sample)} (length: ${history.length})`);
};

/**
 * Delete history for this watch.
 */
export const deleteWatchHistory = () => {
  localStorage.removeItem(buildHistoryKey());
};
