import { HistoryItem } from "./types";

// Same as data.h
export const STATUS_EMPTY = -1;
export const STATUS_NO_CHANGE = -102;
export const STATUS_CHARGED = -103;

/** Watch token was not returned */
export const UPLOAD_ID_EMPTY = 'UPLOAD_ID_EMPTY';
/** Upload API URL */
export const UPLOAD_API_URL = 'https://muninn-api.chrislewis.me.uk';

/** Maximum synced items - 120 days / 4 months (must match config.h) */
export const MAX_SYNC_ITEMS = 480;
/** Seconds in an hour */
export const SECONDS_PER_HOUR = 60 * 60;
/** Seconds in a day */
export const SECONDS_PER_DAY = SECONDS_PER_HOUR * 24;

/** Minimum charge amount to count */
export const MIN_CHARGE_AMOUNT = 20;
/** Max gap in samples */
export const MAX_GAP_SECONDS = 12 * SECONDS_PER_HOUR; // 12 hours

/** Whether to use test data */
export const TEST_SAMPLE_DATA = false;
/** Whether to log successful TS tests */
export const TEST_LOG_OK_TESTS = false;
