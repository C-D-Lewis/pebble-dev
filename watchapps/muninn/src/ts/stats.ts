import {
  STATUS_EMPTY,
  STATUS_CHARGED,
  STATUS_NO_CHANGE,
  SECONDS_PER_DAY,
  MIN_CHARGE_AMOUNT,
  SECONDS_PER_HOUR,
} from './constants';
import { HistoryItem } from './types';

/**
 * Average discharge rate using all history samples.
 */
export const calculateDischargeRate = (history: HistoryItem[]): number => {
  if (history.length === 0) return STATUS_EMPTY;

  let chargeDiffSum = 0;
  let timeDiffSum = 0;
  history.forEach((p) => {
    const { chargeDiff, timeDiff, result } = p;

    if ([STATUS_EMPTY, STATUS_CHARGED].includes(result)) return;

    timeDiffSum += timeDiff;
    chargeDiffSum += chargeDiff;
  });

  if (timeDiffSum <= 0) return STATUS_EMPTY;

  return Math.round((chargeDiffSum * SECONDS_PER_DAY) / timeDiffSum);
};

/**
 * Calculate the average discharge rate for the last week only.
 */
export const calculateLastWeekRate = (history: HistoryItem[]): number => {
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
export const isChargeEvent = (item: HistoryItem) =>
  item.result === STATUS_CHARGED && item.chargeDiff <= (-MIN_CHARGE_AMOUNT);

/**
 * Calculate the number of significant charge events.
 */
export const calculateNumCharges = (history: HistoryItem[]): number =>
  history.filter(isChargeEvent).length;

/**
 * Calculate mean time between charge events, in days.
 */
export const calculateMeanTimeBetweenCharges = (history: HistoryItem[]): number => {
  if (calculateNumCharges(history) < 2) {
    console.log(`WARN: num charges < 2: ${calculateNumCharges(history)}`) ;
    return STATUS_EMPTY;
  }

  // Get timestamps of charge events
  const chargeTimes = history
    .filter(isChargeEvent)
    .map(p => p.timestamp)
    .sort((a, b) => a - b);

  // Calculate time differences between consecutive charges
  const timeDiffs = [];
  for (let i = 1; i < chargeTimes.length; i++) {
    const diff = chargeTimes[i] - chargeTimes[i - 1];
    if (diff < SECONDS_PER_DAY) {
      console.log(`WARN: Ignoring <1d diff (${i} ${i - 1} ${diff}s)`);
      continue;
    }
    timeDiffs.push(diff);
  }

  if (timeDiffs.length === 0) {
    console.log(`WARN: Not enough timeDiffs (${JSON.stringify({ chargeTimes, timeDiffs })})`);
    return STATUS_EMPTY;
  }

  // Calculate mean time between charges (in days)
  const meanTimeS = timeDiffs.reduce((acc, p) => acc + p, 0) / timeDiffs.length;
  const meanDays = meanTimeS / SECONDS_PER_DAY;
  console.log(JSON.stringify({ chargeTimes, timeDiffs, meanTimeS, meanDays }));

  return Math.round(meanDays);
};
