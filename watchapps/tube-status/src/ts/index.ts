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

/** Order lines should be transmitted in */
const LINE_KEY_ORDER = [
  'LineTypeBakerloo',
  'LineTypeCentral',
  'LineTypeCircle',
  'LineTypeDistrict',
  'LineTypeDLR',
  'LineTypeElizabeth',
  'LineTypeHammersmithAndCity',
  'LineTypeJubilee',
  'LineTypeLiberty',
  'LineTypeLioness',
  'LineTypeMetropolitan',
  'LineTypeMildmay',
  'LineTypeNorthern',
  'LineTypePicadilly',
  'LineTypeSuffragette',
  'LineTypeVictoria',
  'LineTypeWaterlooAndCity',
  'LineTypeWeaver',
  'LineTypeWindrush'
];
/** Max reason length */
const MAX_REASON_LENGTH = 256;

/**
 * Download all lines statuses.
 * Available modes: https://api.tfl.gov.uk/StopPoint/Meta/modes
 *
 * @param {string[]} modes Array of transport modes to include.
 */
const downloadStatus = async (modes: string[]): Promise<LineData[]> => {
  const json = await PebbleTS.fetchJSON(`https://api.tfl.gov.uk/line/mode/${modes.join(',')}/status`) as TfLApiResult[];
  // console.log(JSON.stringify(json, null, 2));
  return json.reduce((acc, obj: TfLApiResult): LineData[] => {
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
 * @param {LineData[]} data - All line data.
 * @param {number} index - Item to send.
 */
const sendNextLine = async (data: LineData[], index: number) => {
  if (index === LINE_KEY_ORDER.length) return;

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

  const lineData = await downloadStatus([
    'tube',
    'dlr',
    'elizabeth-line',
    'overground',
  ]);

  try {
    await sendNextLine(lineData, 0);
  } catch (e) {
    console.log('Failed to send data');
    console.log(e);
  }
});
