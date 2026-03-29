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

export type GlobalStatItem = {
  groupName: string;
  names: string[];
  avgBatteryLife: number;
  avgRate: number;
  count: number;
}

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
