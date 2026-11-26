import { LineData, LineId, TfLApiResult } from './types';

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
/** Max reason length - matching C structure */
const MAX_REASON_LENGTH = 512;
/** Good Service status string */
const STATUS_GOOD_SERVICE = 'Good Service';

/**
 * Download all lines statuses.
 * Available modes: https://api.tfl.gov.uk/StopPoint/Meta/modes
 */
const fetchLinesWithIssues = async (): Promise<LineData[]> => {
  const url = `https://api.tfl.gov.uk/line/mode/${MODES.join(',')}/status`;
  const json = await PebbleTS.fetchJSON(url) as TfLApiResult[];
  // console.log(JSON.stringify(json, null, 2));

  // Test empty response - no issues
  // return [];

  // Send only those with issues (will need better logic if a setting for this is implemented)
  return json
    .filter(
      (p: TfLApiResult) => p.lineStatuses[0].statusSeverityDescription !== STATUS_GOOD_SERVICE,
    )
    .reduce((acc, p: TfLApiResult): LineData[] => {
      let reason = p.lineStatuses[0].reason || '';
      if (reason?.length > MAX_REASON_LENGTH) {
        reason = reason?.substring(0, MAX_REASON_LENGTH - 4) + '...';
      }

      const item = {
        id: p.id,
        status: p.lineStatuses[0].statusSeverityDescription || '?',
        reason,
      };
      // console.log(item);
      return [...acc, item];
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
      FLAG_IS_COMPLETE: 1,
      FLAG_LINE_COUNT: 0,
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
    LINE_TYPE: LINE_TYPE_MAP[lineData.id as LineId],
    LINE_STATUS: lineData.status,
    LINE_REASON: lineData.reason,
    FLAG_IS_COMPLETE: index === lines.length - 1 ? 1 : 0,
    FLAG_LINE_COUNT: lines.length,
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
