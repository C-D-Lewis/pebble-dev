export type HistoryItem = {
  ts: number;
  cp: number;
  r: number;
  res: number;
};

export type ApiGwEvent = {
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

export type EventBridgeEvent = {
  'detail-type': 'DailyAggregation';
  source: 'aws.events';
};

export type WatchStats = {
  count: number;
  totalDays: number;
  allTimeRate: number;
  lastWeekRate: number;
  numCharges: number;
  mtbc: number;
};

/** Data from a single watch */
export type WatchData = {
  history: HistoryItem[];
  platform: string;
  model: string;
  firmware: string;
  stats: WatchStats;
};

export type DbDocument = {
  [key: string];

  id: string;
  updatedAt: number;
} & WatchData;

export type PostIdBody = {
  watchToken: string;
};

export type PostHistoryBody = {
  id: string;
} & WatchData;

export type GetHistoryResponse = {
  updatedAt: number;
} & WatchData;

type StatItem = {
  groupName: string;
  names: string[];
  count: number;
  
  minBatteryLife: number;
  maxBatteryLife: number;
  batteryLifeRange: number;
  values: number[];
};

export type AggregateItem = StatItem & {
  totalBatteryLife: number;
  totalRate: number;
};

export type GlobalStatItem = StatItem & {
  avgBatteryLife: number;
  avgRate: number;

  medianBatteryLife: number;
};

export type GetGlobalStatsResponse = {
  historyCount: number;
  models: GlobalStatItem[];
  platforms: GlobalStatItem[];
  updatedAt: number;
};

export type LambdaResponse = {
  statusCode: number;
  body: string;
  headers?: Record<string, string>;
};

export type MetadataDocument = {
  id: AGGREGATION_DOC_ID;
} & GetGlobalStatsResponse;
