/** Expected Line ID values */
export type LineId = 'bakerloo' | 'central' | 'circle' | 'district' | 'dlr' | 'elizabeth' |
  'hammersmith-city' | 'jubilee' | 'liberty' | 'lioness' | 'metropolitan' | 'mildmay' |
  'northern' | 'piccadilly' | 'suffragette' | 'victoria' | 'waterloo-city' | 'weaver' |
  'windrush';

/** API data type */
export type TfLApiResult = {
  id: LineId;
  lineStatuses: {
    statusSeverityDescription: string;
    reason?: string;
  }[];
};

/** Processed data type */
export type LineData = {
  id: string;
  status: string;
  reason: string;
};
