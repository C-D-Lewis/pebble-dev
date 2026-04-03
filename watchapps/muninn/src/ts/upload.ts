import { UPLOAD_ID_EMPTY, UPLOAD_API_URL } from './constants';
import {
  calculateDischargeRate,
  calculateLastWeekRate,
  calculateNumCharges,
  calculateMeanTimeBetweenCharges,
} from './stats';
import { loadHistory } from './sync';
import { HistoryItem, UploadHistoryItem } from './types';
import { buildUploadKey } from './util';

const mapHistoryForDisplay = (history: HistoryItem[]): UploadHistoryItem[] =>
  history.map(p => ({
    ts: p.timestamp,
    cp: p.chargePerc,
    r: p.rate,
    res: p.result,
  }));


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
};

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
 * Upload history to the Muninn API for display and sharing.
 */
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
  if (!history.length) {
    console.log('No history, nothing to upload');
    return;
  }

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
