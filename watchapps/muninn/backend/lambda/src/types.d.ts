export type HistoryItem = {
  ts: number;
  cp: number;
  r: number;
  res: number;
};

export type LambdaEvent = {
  httpMethod: 'POST' | 'GET';
  routeKey: string;
  body?: string;
  pathParameters?: {
    id: string;
  };
  headers: {
    origin?: string;
  };
};

export type Stats = {
  count: number;
  totalDays: number;
  allTimeRate: number;
  lastWeekRate: number;
  numCharges: number;
  mtbc: number;
};

export type PostIdBody = {
  watchToken: string;
};

export type PostHistoryBody = {
  id: string;
  history: HistoryItem[];
  platform: string;
  model: string;
  firmware: string;
  stats: Stats;
};

export type GetHistoryResponse = {
  history: HistoryItem[];
  platform: string;
  model: string;
  firmware: string;
  stats: Stats;
};

export type GetGlobalStatsResponse = {
  totalUploads: number;
};

export type LambdaResponse = {
  statusCode: number;
  body: string;
  headers?: Record<string, string>;
};
