export type HttpMethod = 'POST' | 'PUT' | 'GET' | 'DELETE';

export type HistoryItem = {
  timestamp: number;
  result: number;
  chargePerc: number;
  timeDiff: number;
  chargeDiff: number;
  rate: number;
};

export type UploadHistoryItem = {
  ts: number;
  res: number;
  cp: number;
  r: number;
};
