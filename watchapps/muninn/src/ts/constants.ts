import { HistoryItem } from "./types";

// Same as data.h
export const STATUS_EMPTY = -1;
export const STATUS_NO_CHANGE = -102;
export const STATUS_CHARGED = -103;

/** Maximum stored items - 32 days */
export const MAX_ITEMS = 128;
/** Seconds in a day */
export const SECONDS_PER_DAY = 60 * 60 * 24;
/** Minimum charge amount to count */
export const MIN_CHARGE_AMOUNT = 30;

/** Whether to use test data */
export const TEST_SAMPLE_DATA = false;
