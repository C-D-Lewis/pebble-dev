import { STATUS_CHARGED, STATUS_EMPTY, STATUS_NO_CHANGE } from './constants';
import { HistoryItem } from './types';

/** Maximum stored items - 32 days */
const MAX_ITEMS = 128;

/** LocalStorage key - data history on a per-watch basis */
const buildHistoryKey = () => `history-${Pebble.getWatchToken()}`;

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
  try {
    return JSON.parse(localStorage.getItem(buildHistoryKey()) || '[]');
  } catch (e) {
    console.error('Failed to load history');
    console.error(e);
    throw e;
  }
};

/**
 * Send the watch the last seen timestamp so updates can be incremental.
 */
export const handleGetSyncInfo = async () => {
  const history = loadHistory();
  // console.log(JSON.stringify(localStorage.getItem('history')));

  let timestamp = STATUS_EMPTY;
  if (history.length > 0) {
    timestamp = history[0].timestamp;
  }

  await PebbleTS.sendAppMessage({
    SYNC_TIMESTAMP: timestamp,
    SYNC_COUNT: history.length
  });
};

/**
 * Importing data should be as robust as possible.
 *
 * TODO: How do we handle adding new fields?
 */
export const handleSync = async (dict: Record<string, any>) => {
  let history = loadHistory();
  const timestamp = dict.SAMPLE_TIMESTAMP;

  // Manual loop replaces .find() which coreapp doesn't like
  for (let i = 0; i < history.length; i++) {
    if (history[i].timestamp === timestamp) {
      console.log(`Skipping duplicate: ${timestamp}`);
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
  console.log(`Saved new sample: ${JSON.stringify(sample)}`);
};

/**
 * Delete history for this watch.
 */
export const deleteWatchHistory = () => {
  localStorage.removeItem(buildHistoryKey());
};

/**
 * Average discharge rate using all history samples.
 * Because this represents per-day and counts 6H samples, we multiply by 4 
 */
const calculateAllTimeRate = (history: HistoryItem[]): number => {
  if (history.length === 0) return STATUS_EMPTY;

  let samples = 0;
  let change = 0;
  history.forEach((item) => {
    const { chargeDiff, result } = item;

    // No data, or charged.
    if ([STATUS_EMPTY, STATUS_CHARGED].includes(result)) return;
    // No change, but time still elapsed
    if (result === STATUS_NO_CHANGE) {
      samples++;
      return;
    }

    // Discharged
    samples++;
    change += Math.abs(chargeDiff);
  });
  const result = Math.round(change / samples) * 4;
  console.log(JSON.stringify({ samples, change, result }));
  return result;
};

/**
 * Handle request for stats based on all data stored for this watch.
 * TODO: Future graph points?
 */
export const handleGetSyncStats = async () => {
  const history = loadHistory();

  await PebbleTS.sendAppMessage({
    STAT_TOTAL_DAYS: Math.floor(history.length / 4),
    STAT_ALL_TIME_RATE: calculateAllTimeRate(history),
    // STAT_LAST_WEEK_RATE: calculateLastWeekRate(history),
    // STAT_NUM_CHARGES: calculateNumCharges(history),
    // STAT_MTBC: calculateMeanTimeBetweenCharges(history),
  });
};
