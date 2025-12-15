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

/** Despite attempts to not rely on index, need to map these API IDs to C enum values */
const LINE_TYPE_MAP: { [key in LineId]: number } = {
  bakerloo: 0,
  central: 1,
  circle: 2,
  district: 3,
  dlr: 4,
  elizabeth: 5,
  'hammersmith-city': 6,
  jubilee: 7,
  liberty: 8,
  lioness: 9,
  metropolitan: 10,
  mildmay: 11,
  northern: 12,
  piccadilly: 13,
  suffragette: 14,
  victoria: 15,
  'waterloo-city': 16,
  weaver: 17,
  windrush: 18,
};

/** API modes to query */
const MODES = ['tube', 'dlr', 'elizabeth-line', 'overground'];
/** Number of lines the API returns */
const NUM_LINES = 19;
/** Max reason length */
const MAX_REASON_LENGTH = 512;

/**
 * Download all lines statuses.
 * Available modes: https://api.tfl.gov.uk/StopPoint/Meta/modes
 */
const fetchLinesWithIssues = async (): Promise<LineData[]> => {
  const url = `https://api.tfl.gov.uk/line/mode/${MODES.join(',')}/status`;
  const json = await PebbleTS.fetchJSON(url) as TfLApiResult[];
  // console.log(JSON.stringify(json, null, 2));

  // Send only those with issues (will need better logic if a setting for this is implemented)
  // return [];
  return json
    .filter((obj: TfLApiResult) => obj.lineStatuses[0].statusSeverityDescription !== 'Good Service')
    .reduce((acc, obj: TfLApiResult): LineData[] => {
      let reason = obj.lineStatuses[0].reason || '';
      if (reason?.length > MAX_REASON_LENGTH) {
        reason = reason?.substring(0, MAX_REASON_LENGTH - 4) + '...';
      }

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
 * @param {LineData[]} lines - Data for all lines with issues.
 * @param {number} index - Item to send.
 */
const sendNextLine = async (lines: LineData[], index: number) => {
  // Everything is awesome!
  if (lines.length === 0) {
    const dict = {
      FlagIsComplete: 1,
      FlagLineCount: 0,
    };
    await PebbleTS.sendAppMessage(dict);
    return;
  }

  // Completed
  if (index === lines.length) {
    console.log('All data sent!');
    return;
  }

  const lineData = lines[index];
  if (!lineData) throw new Error(`No lineData for ${index}`);

  const dict = {
    LineIndex: index,
    LineType: LINE_TYPE_MAP[lineData.id as LineId],
    LineStatus: lineData.status,
    LineReason: lineData.reason,
    FlagIsComplete: index === lines.length - 1 ? 1 : 0,
    FlagLineCount: lines.length,
  };
  await PebbleTS.sendAppMessage(dict);
  console.log(`Sent item ${index}: ${JSON.stringify(dict)}`);

  await sendNextLine(lines, index + 1);
};

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready');

  try {
    const lines = await fetchLinesWithIssues();
    await sendNextLine(lines, 0);
  } catch (e) {
    console.log('Failed to send data');
    console.log(e);
  }
});
