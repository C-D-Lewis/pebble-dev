import { ALLOWED_ORIGINS, DEFAULT_RES_HEADERS } from './constants.ts';
import type { HistoryItem, ApiGwEvent, PostHistoryBody, WatchStats, GlobalStatItem } from './types.ts';

/**
 * Generate a random 6-character hexadecimal ID.
 * 16677715 is 0xFFFFFF, the max for 6 hex digits.
 *
 * @return {string} The generated ID.
 */
export const generateId = () =>
  Math.floor(Math.random() * 16777215)
    .toString(16)
    .padStart(6, '0')
    .toUpperCase();

/**
 * Create a bad request response.
 *
 * @param {string} message - The error message (logged only).
 * @return {object} The response object.
 */
export const badRequest = (message: string) => {
  if (!process.env.NODE_ENV) console.warn(`Bad request: ${message}`);
  return {
    statusCode: 400,
    body: JSON.stringify({ error: 'Bad Request' }),
    headers: DEFAULT_RES_HEADERS,
  };
};

/**
 * Create a not found response.
 *
 * @param {string} message - The error message (logged only).
 * @return {object} The response object.
 */
export const notFound = (message: string, headers: any = {}) => {
  if (!process.env.NODE_ENV) console.log(`Not found: ${message}`);
  return {
    statusCode: 404,
    headers: {
      ...DEFAULT_RES_HEADERS,
      ...headers,
    },
    body: JSON.stringify({ error: 'Not Found' }),
  };
};

/**
 * Create an error response.
 *
 * @param {string} message - The error message (logged only).
 * @return {object} The response object.
 */
export const error = (message: string) => {
  console.error(`Error: ${message}`);
  return {
    statusCode: 500,
    headers: DEFAULT_RES_HEADERS,
    body: JSON.stringify({ error: 'Unknown Error' }),
  };
};

/**
 * Create a success response.
 *
 * @param {any} data - Response data.
 * @param {any} headers - Optional headers.
 * @return {object} The response object.
 */
export const success = (data: any, headers: any = {}) => ({
  statusCode: 200,
  body: JSON.stringify(data),
  headers: { ...DEFAULT_RES_HEADERS, ...headers },
});

/**
 * Validate the history data format.
 *   Should sync with JS and client.
 *
 * @param {HistoryItem[]} history - The history data to validate.
 * @returns {boolean} - True if valid, false otherwise.
 */
const validateHistory = (history: HistoryItem[]) => {
  return history.every(p => {
    return typeof p.ts === 'number' &&
      typeof p.cp === 'number' &&
      typeof p.r === 'number' &&
      typeof p.res === 'number';
  });
};

/**
 * Validate the stats data format.
 *   Should sync with JS and client.
 *
 * @param {WatchStats} stats - The stats data to validate.
 * @returns {boolean} - True if valid, false otherwise.
 */
const validateWatchStats = (stats: WatchStats) => {
  return typeof stats.count === 'number' &&
    typeof stats.totalDays === 'number' &&
    typeof stats.allTimeRate === 'number' &&
    typeof stats.lastWeekRate === 'number' &&
    typeof stats.numCharges === 'number' &&
    typeof stats.mtbc === 'number';
};

export const validatePostHistoryBody = (body: PostHistoryBody) => {
  const { id, history, platform, model, firmware, stats } = body;

  if (!id || typeof id !== 'string' || id.length !== 6) return false;
  if (!platform || typeof platform !== 'string') return false;
  if (!model || typeof model !== 'string') return false;
  if (!firmware || typeof firmware !== 'string') return false;
  if (!validateHistory(history)) return false;
  if (!validateWatchStats(stats)) return false;

  return true;
};

export const cors = (event: ApiGwEvent) => {
  const origin = event.headers.origin || '';
  const acaoHeader = ALLOWED_ORIGINS.includes(origin) ? origin : ALLOWED_ORIGINS[1];

  return { 'Access-Control-Allow-Origin': acaoHeader };
};

export const sortByCount = (a: GlobalStatItem, b: GlobalStatItem) => b.count - a.count;

export const sortByName = (a: GlobalStatItem, b: GlobalStatItem) => a.groupName.localeCompare(b.groupName);
