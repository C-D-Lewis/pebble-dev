import { DEFAULT_RES_HEADERS } from './constants.js';
import type { HistoryItem } from './types.js';

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
  console.warn(`Bad request: ${message}`);
  return { statusCode: 400 };
};

/**
 * Create an error response.
 *
 * @param {string} message - The error message (logged only).
 * @return {object} The response object.
 */
export const error = (message: string) => {
  console.error(`Error: ${message}`);
  return { statusCode: 500 };
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
export const validateHistory = (history: HistoryItem[]) => {
  return history.every(p => {
    return typeof p.timestamp === 'number' &&
      typeof p.chargePerc === 'number' &&
      typeof p.rate === 'number' &&
      typeof p.result === 'number';
  });
};
