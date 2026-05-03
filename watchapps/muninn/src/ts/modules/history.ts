import { TEST_SAMPLE_DATA } from '../constants';
import { HistoryItem } from '../types';
import { buildHistoryKey, generateTestData } from './util';

/**
 * Save the history of samples.
 */
export const saveHistory = (history: HistoryItem[]) => {
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
