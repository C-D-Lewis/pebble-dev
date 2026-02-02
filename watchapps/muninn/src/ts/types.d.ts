export type HttpMethod = 'POST' | 'PUT' | 'GET' | 'DELETE';

export type HistoryItem = {
  timestamp: number;
  result: number;
  chargePerc: number;
  timeDiff: number;
  chargeDiff: number;
  rate: number;
};
