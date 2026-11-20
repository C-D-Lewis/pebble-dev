export type HttpMethod = 'POST' | 'PUT' | 'GET' | 'DELETE';

/**
 * Item of sample history received from the watch.
 */
export type HistoryItem = {
  timestamp: number;
  result: number;
  chargePerc: number;
  timeDiff: number;
  chargeDiff: number;
  rate: number;
};

/**
 * Abrieviated sample history item uploaded to the config page API and stored
 * in a DynamoDB, which charges and pages per byte read.
 */
export type UploadHistoryItem = {
  ts: number;
  res: number;
  cp: number;
  r: number;
};

/**
 * Shape of stats provided by the Pebble Plugin.
 */
export type PluginStats = {
  /** Number of estimated days remaining */
  daysRemaining: number;
  /** Battery life in days */
  batteryLife: number;
  /** Rate discharged over last week only */
  lastWeekRate: number;
  /** When stats were updated */
  updatedAt: number;
};
