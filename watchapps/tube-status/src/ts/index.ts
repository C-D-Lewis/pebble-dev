/** Expected Line ID values */
type LineId = 'bakerloo' | 'central' | 'circle' | 'district' | 'dlr' | 'elizabeth' |
  'hammersmith-city' | 'jubilee' | 'liberty' | 'lioness' | 'metropolitan' | 'mildmay' |
  'northern' | 'piccadilly' | 'suffragette' | 'victoria' | 'waterloo-city' | 'weaver' |
  'windrush';

  /** API data type */
type TfLApiResult = {
  id: LineId;
  lineStatuses: {
    statusSeverityDescription: string;
    reason?: string;
  }[];
};

/** Processed data type */
type LineData = {
  id: string;
  status: string;
  reason: string;
};

/** API modes to query */
const MODES = ['tube', 'dlr', 'elizabeth-line', 'overground'];
/** Number of lines the API returns */
const NUM_LINES = 19;
/** Max reason length */
const MAX_REASON_LENGTH = 256;

/**
 * Download all lines statuses.
 * Available modes: https://api.tfl.gov.uk/StopPoint/Meta/modes
 */
const downloadStatus = async (): Promise<LineData[]> => {
  const url = `https://api.tfl.gov.uk/line/mode/${MODES.join(',')}/status`;
  const json = await PebbleTS.fetchJSON(url) as TfLApiResult[];
  // console.log(JSON.stringify(json, null, 2));

  return json.reduce((acc, obj: TfLApiResult): LineData[] => {
    let reason = obj.lineStatuses[0].reason || '';
    if (reason?.length > MAX_REASON_LENGTH) {
      reason = reason?.substring(0, MAX_REASON_LENGTH - 4) + '...';
    }

    // Order is very important and must match C side
    return [
      ...acc,
      {
        id: obj.id,
        status: obj.lineStatuses[0].statusSeverityDescription || '?',
        reason,
      },
    ];
  }, []);
};

/**
 * Send next line's data.
 *
 * @param {LineData[]} data - All line data.
 * @param {number} index - Item to send.
 */
const sendNextLine = async (data: LineData[], index: number) => {
  if (index === NUM_LINES) {
    console.log('All data sent!');
    return;
  }

  const lineData = data[index];
  if (!lineData) throw new Error(`No lineData for ${index}`);

  const dict = {
    LineIndex: index,
    LineStatus: lineData.status,
    LineReason: lineData.reason,
  };
  await PebbleTS.sendAppMessage(dict);
  // console.log(`Sent item ${index}`);

  await sendNextLine(data, index + 1);
};

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready');

  const lineData = await downloadStatus();

  try {
    await sendNextLine(lineData, 0);
  } catch (e) {
    console.log('Failed to send data');
    console.log(e);
  }
});
