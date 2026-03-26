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

/** Global API data */
export type GlobalStats = {
  historyCount: number;
};

/** Chart modes */
export type ChartMode = 'all' | 'lastWeek' | 'lastMonth';

/** State of the fabricate.js app */
export type AppState = {
  // App state
  loading: boolean;
  notFound: boolean;
  chartMode: ChartMode;

  // User API data
  id: string;
  updatedAt: number;
  history: HistoryItem[];
  platform: string;
  model: string;
  firmware: string;
  stats: Stats;

  // Global API data
  globalStats: GlobalStats;
};
