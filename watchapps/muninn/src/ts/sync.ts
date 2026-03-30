import {
  MAX_SYNC_ITEMS,
  STATUS_EMPTY,
  TEST_SAMPLE_DATA,
  UPLOAD_API_URL,
  UPLOAD_ID_EMPTY,
} from './constants';
import {
  calculateDischargeRate,
  calculateLastWeekRate,
  calculateNumCharges,
  calculateMeanTimeBetweenCharges,
} from './stats';
import { HistoryItem, UploadHistoryItem } from './types';
import { generateTestData } from './util';

/**
 * LocalStorage key - data history on a per-watch basis
 */
const buildHistoryKey = () => {
  const key = `history-${Pebble.getWatchToken()}`;
  // console.log(JSON.stringify({ key }));
  return key;
};

/**
 * LocalStorage key - upload ID on a per-watch basis
 */
const buildUploadKey = () => {
  const uploadKey = `upload-${Pebble.getWatchToken()}`;
  // console.log(JSON.stringify({ uploadKey }));
  return uploadKey;
};

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
const loadHistory = (): HistoryItem[] => {
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
 * Get upload ID from the API, if possible.
 *
 * @returns Upload ID, or constant indicating no ID available. 
 */
const getUploadId = async (): Promise<string> => {
  const watchToken = Pebble.getWatchToken();
  if (!watchToken || watchToken.length === 0) {
    console.log('WARN: No watchToken was returned');
    return UPLOAD_ID_EMPTY;
  }

  const res = await fetch(`${UPLOAD_API_URL}/id`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ watchToken }),
  });
  if (res.status !== 200) {
    console.log(`Failed to get upload id`);
    console.log(await res.text());
    return UPLOAD_ID_EMPTY;
  }
  
  const { id = UPLOAD_ID_EMPTY } = await res.json();
  return id;
}

/**
 * Ensure the upload ID is ready if possible.
 */
export const ensureUploadId = async () => {
  let uploadId = localStorage.getItem(buildUploadKey());
  if (!uploadId || uploadId === UPLOAD_ID_EMPTY) {
    uploadId = await getUploadId();
  }
  localStorage.setItem(buildUploadKey(), uploadId);
  return uploadId;
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

const mapHistoryForDisplay = (history: HistoryItem[]): UploadHistoryItem[] =>
  history.map(p => ({
    ts: p.timestamp,
    cp: p.chargePerc,
    r: p.rate,
    res: p.result,
  }));

export const uploadHistory = async () => {
  const id = localStorage.getItem(buildUploadKey());
  if (!id || id === UPLOAD_ID_EMPTY) {
    console.log('WARN: No upload ID, cannot upload');
    await PebbleTS.sendAppMessage({ UPLOAD_STATUS: 0 });
    return;
  }

  const watchInfo = Pebble.getActiveWatchInfo();
  const platform = watchInfo.platform || 'unknown';
  const model = watchInfo.model || 'unknown';
  const firmware = !watchInfo.firmware
    ? 'unknown'
    : `${watchInfo.firmware.major}.${watchInfo.firmware.minor}.${watchInfo.firmware.patch}`;

  const history = loadHistory();

  const stats = {
    count: history.length,
    totalDays: Math.floor(history.length / 4),
    allTimeRate: calculateDischargeRate(history),
    lastWeekRate: calculateLastWeekRate(history),
    numCharges: calculateNumCharges(history),
    mtbc: calculateMeanTimeBetweenCharges(history),
  };

  const res = await fetch(`${UPLOAD_API_URL}/history`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      id,
      history: mapHistoryForDisplay(history),
      platform,
      model,
      firmware,
      stats,
    }),
  });
  if (res.status !== 200) {
    console.log(`WARN: Failed to upload history`);
    console.log(await res.text());
    await PebbleTS.sendAppMessage({ UPLOAD_STATUS: 0 });

    // In case the DB was erased, get it again
    await ensureUploadId();
    return;
  }

  console.log(`Uploaded ${history.length} history items`);
  await PebbleTS.sendAppMessage({ UPLOAD_STATUS: 1 });
};
