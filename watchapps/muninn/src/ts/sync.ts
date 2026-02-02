import { HistoryItem } from './types';

/** Maximum stored items */
const MAX_ITEMS = 64;
/** LocalStorage key - data history */
const LS_KEY_HISTORY = 'history';

/**
 * Save the history of samples.
 */
const saveHistory = (history: HistoryItem[]) => {
  localStorage.setItem(LS_KEY_HISTORY, JSON.stringify(history));
};

/**
 * Load the history of samples.
 */
const loadHistory = (): HistoryItem[] => {
  try {
    return JSON.parse(localStorage.getItem(LS_KEY_HISTORY) || '[]');
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

  let timestamp = -1; // STATUS_EMPTY
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
      console.log('Skipping duplicate: ' + timestamp);
      return;
    }
  }

  // Construct local sample
  const sample: HistoryItem = {
    timestamp: dict.SAMPLE_TIMESTAMP,
    result: dict.SAMPLE_RESULT,
    chargePerc: dict.SAMPLE_CHARGE_PERC,
    timeDiff: dict.SAMPLE_TIME_DIFF,
    chargeDiff: dict.SAMPLE_CHARGE_DIFF,
    rate: dict.SAMPLE_RATE,
  };
  history.push(sample);

  // Sort to match watch order and limit length
  history = history
    .sort((a, b) => a.timestamp > b.timestamp ? -1 : 1)
    .slice(0, MAX_ITEMS);

  saveHistory(history);
  console.log('Saved new sample: ' + JSON.stringify(sample));
};
