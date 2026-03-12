/** Single item of history, after upload mapping */
export type HistoryItem = {
  ts: number;
  cp: number;
  r: number;
  res: number;
};

/** Stats derived by the JS side */
export type Stats = {
  count: number;
  totalDays: number;
  allTimeRate: number;
  lastWeekRate: number;
  numCharges: number;
  mtbc: number;
};

export type GlobalStats = {
  totalUploads: number;
}

/** State of the fabricate.js app */
export type AppState = {
  loading: boolean;
  notFound: boolean;
  id: string;
  history: HistoryItem[];
  platform: string;
  model: string;
  firmware: string;
  stats: Stats;
  globalStats: GlobalStats;
};
