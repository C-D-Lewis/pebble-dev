type TfLStatus = {
  id: string;
  lineStatuses: {
    statusSeverity: number;
    statusSeverityDescription: string;
    reason?: string;
  }[];
};

type LineStates = {
  [lineId: string]: string;
}

/**
 * Download all lines statuses.
 * Available modes: https://api.tfl.gov.uk/StopPoint/Meta/modes
 *
 * @param {string[]} modes Array of transport modes to include.
 */
const downloadStatus = async (modes: string[]) =>  {
  const json = await PebbleTS.fetchJSON(`https://api.tfl.gov.uk/line/mode/${modes.join(',')}/status`) as TfLStatus[];
  return json.reduce((acc, obj: TfLStatus) => {
    return {
      ...acc,
      [obj.id]: obj.lineStatuses[0].statusSeverityDescription || '?',
    };
  }, {});
};

/**
 * Send all lines statuses to Pebble.
 *
 * @param {Object} lineStates Object mapping line IDs to status strings.
 */
const sendToPebble = async (lineStates: LineStates) => {
  // TODO: Stream messages with larger lineStatuses[n].reason string for detail window
  const dict = {
    'LineTypeBakerloo': lineStates['bakerloo'] || '?',
    'LineTypeCentral': lineStates['central'] || '?',
    'LineTypeCircle': lineStates['circle'] || '?',
    'LineTypeDistrict': lineStates['district'] || '?',
    'LineTypeDLR': lineStates['dlr'] || '?',
    'LineTypeElizabeth': lineStates['elizabeth'] || '?',
    'LineTypeHammersmithAndCity': lineStates['hammersmith-city'] || '?',
    'LineTypeJubilee': lineStates['jubilee'] || '?',
    'LineTypeLiberty': lineStates['liberty'] || '?',
    'LineTypeLioness': lineStates['lioness'] || '?',
    'LineTypeMetropolitan': lineStates['metropolitan'] || '?',
    'LineTypeMildmay': lineStates['mildmay'] || '?',
    'LineTypeNorthern': lineStates['northern'] || '?',
    'LineTypePicadilly': lineStates['piccadilly'] || '?',
    'LineTypeSuffragette': lineStates['suffragette'] || '?',
    'LineTypeVictoria': lineStates['victoria'] || '?',
    'LineTypeWaterlooAndCity': lineStates['waterloo-city'] || '?',
    'LineTypeWeaver': lineStates['weaver'] || '?',
    'LineTypeWindrush': lineStates['windrush'] || '?'
  };

  await PebbleTS.sendAppMessage(dict);
}

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready');

  const lineStates = await downloadStatus([
    'tube',
    'dlr',
    'elizabeth-line',
    'overground',
  ]);

  await sendToPebble(lineStates);
});
