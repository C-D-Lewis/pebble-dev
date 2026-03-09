export type HistoryItem = {
  timestamp: number;
  chargePerc: number;
  rate: number;
  result: number;
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

export type PostIdBody = {
  watchToken: string;
};

export type PostHistoryBody = {
  id: string;
  history: HistoryItem[];
  platform: string;
  model: string;
  firmware: string;
};

export type GetHistoryResponse = {
  history: HistoryItem[];
  platform: string;
  model: string;
  firmware: string;
};

export type GetStatsResponse = {
  totalIds: number;
  totalUploads: number;
};

export type LambdaResponse = {
  statusCode: number;
  body: string;
  headers?: Record<string, string>;
};
