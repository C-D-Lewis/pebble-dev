import { type TransitBackend, type LineConfig, type GenericLineData, londonUndergroundBackend } from './backends';

// TODO: Configure ACTIVE_BACKEND with settings in the future
const ACTIVE_BACKEND: TransitBackend = londonUndergroundBackend;

/**
 * Send line configurations to the watch
 */
const sendLineConfigs = async (configs: LineConfig[]): Promise<void> => {
  console.log(`Sending ${configs.length} line configurations...`);

  for (const config of configs) {
    const dict = {
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

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready');
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
});
