import { type LineConfig, type GenericLineData, jrEastKantoBackend, londonUndergroundBackend, nycMtaSubwayBackend, TransitBackend } from './backends';

// Order can be changed, Tube Status uses -1 to indicate London Underground
const TransitBackendIdMap: Record<number, TransitBackend> = {
  0: londonUndergroundBackend,
  1: jrEastKantoBackend,
  2: nycMtaSubwayBackend
} as const;

let ACTIVE_BACKEND: TransitBackend = londonUndergroundBackend;

/**
 * Send line configurations to the watch
 */
const sendLineConfigs = async (configs: LineConfig[]): Promise<void> => {
  console.log(`Sending ${configs.length} line configurations...`);

  for (const config of configs) {
    const dict = {
      Type: 'lineConfig',
      ConfigLineIndex: config.index,
      ConfigLineName: config.name,
      ConfigLineColor: config.color,
      ConfigLineStriped: config.striped ? 1 : 0,
    };
    await PebbleTS.sendAppMessage(dict);
    console.log(`Configured line ${config.index}: ${config.name}`);
  }

  console.log('All line configurations sent!');
};

/**
 * Send all status updates with completion flags
 */
const sendLineStatuses = async (lines: GenericLineData[]): Promise<void> => {
  // Everything is awesome!
  if (lines.length === 0) {
    const dict = {
      Type: 'lineStatus',
      FlagIsComplete: 1,
      FlagLineCount: 0,
    };
    await PebbleTS.sendAppMessage(dict);
    return;
  }

  // Send each line status
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const dict = {
      Type: 'lineStatus',
      LineStatusIndex: i,
      LineStatus: line.status,
      LineStatusSeverity: line.severity,
      LineReason: line.reason,
      FlagIsComplete: i === lines.length - 1 ? 1 : 0,
      FlagLineCount: lines.length,
    };
    await PebbleTS.sendAppMessage(dict);
    console.log(`Sent status ${i + 1}/${lines.length}: Line ${line.index} -> Display index ${i}`);
  }

  console.log('All status updates sent!');
};

/**
 * Send all available transit systems to the watch with completion flag
 */
const sendAvailableTransitSystems = async (transitSystems: Record<number, TransitBackend>) => {
  const transitSystemBackendTotal = Object.keys(transitSystems).length;
  Object.entries(transitSystems).forEach(async ([id, backend]) => {
    const dict = {
      Type: 'availableTransitSystem',
      AvailableTransitSystemIndex: parseInt(id),
      AvailableTransitSystemName: backend.name,
      AvailableTransitSystemRegion: backend.region,
      FlagLineCount: transitSystemBackendTotal,
      FlagIsComplete: parseInt(id) === (transitSystemBackendTotal - 1) ? 1 : 0,
    };
    await PebbleTS.sendAppMessage(dict);
    console.log(`Sent available transit system: ${backend.name} (ID: ${id})`);
  });
};

Pebble.addEventListener('ready', async (_) => {
  console.log('PebbleKit JS ready');
  PebbleTS.sendAppMessage({ Type: 'ready' });
});

Pebble.addEventListener('appmessage', async (e) => {
  if (!e.payload) {
    return;
  }

  console.log(JSON.stringify(e.payload));
  if (e.payload.RequestAvailableTransitSystems) {
    sendAvailableTransitSystems(TransitBackendIdMap);
    return;
  }

  // This is a 0 index, possible negative, based ID so we should not truthy check it
  if (e.payload.RequestTransitSystem !== undefined) {
    if (e.payload.RequestTransitSystem === -1) {
      ACTIVE_BACKEND = londonUndergroundBackend;
    } else {
      ACTIVE_BACKEND = TransitBackendIdMap[e.payload.RequestTransitSystem];
      if (!ACTIVE_BACKEND) {
        console.log(`No backend found for transit system ID: ${e.payload.RequestTransitSystem}`);
        ACTIVE_BACKEND = londonUndergroundBackend;
      }
    }

    console.log(`Active backend: ${ACTIVE_BACKEND.name}`);

    try {
      // Fetch lines with issues first
      const lines = await ACTIVE_BACKEND.fetchLines();

      // Only send configs for lines that have issues
      if (lines.length > 0) {
        const allConfigs = ACTIVE_BACKEND.getLineConfigs();

        const neededConfigs = lines.configMapping.map((configIndex: number, displayIndex: number) => {
          const config = allConfigs[configIndex];
          return { ...config, index: displayIndex }; // Remap to sequential index
        });

        console.log(`Sending configs for ${neededConfigs.length} lines with issues (out of ${allConfigs.length} total)`);
        await sendLineConfigs(neededConfigs);
      } else {
        console.log('No issues detected, skipping line configurations');
      }

      // Send status updates
      await sendLineStatuses(lines);
    } catch (e) {
      console.log('Failed to send data');
      console.log(e);
    }
  }
});
