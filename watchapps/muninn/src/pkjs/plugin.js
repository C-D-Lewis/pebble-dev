// TODO: Move this to PKTS when able - manifest requires source file name...

/** LocalStorage key for stats */
const STORAGE_KEY_PLUGIN_STATS = 'PLUGIN_STATS';

/**
 * Plugin to expose derived statistics from Muninn's calculations.
 * If the user hasn't enough data yet, returns nothing useful.
 * 
 * Shape defined in types.d.ts
 */
Pebble.registerSourceHandler(async (req, res) => {
  const { category, item } = req;
  if (category !== 'muninn' || item !== 'stats') {
    console.log(`Invalid request: ${category}/${item}`);
    res.error('PLUGIN_UNAVAILABLE');
    return;
  }

  // Get stats
  const stats = JSON.parse(localStorage.getItem(STORAGE_KEY_PLUGIN_STATS) || '{}');
  console.log(`Loaded muninn/stats: ${JSON.stringify(stats)}`);

  // Return shape expected by mobile app
  res.data({
    validUntilMs: Date.now() + 60_000, // Match refresh interval in manifest
    instances: [{
      instanceId: '0', // Must be string!
      // DO NOT ALTER THIS INTERFACE WITHOUT A GOOD PLAN
      properties: {
        daysRemaining: {
          numericValue: { value: stats.daysRemaining || -1, unit: 'days' },
        },
        batteryLife: {
          numericValue: { value: stats.batteryLife || -1, unit: 'days' },
        },
        lastWeekRate: {
          numericValue: { value: stats.lastWeekRate || -1, unit: '%' },
        },
        updatedAt: {
          numericValue: { value: stats.updatedAt || -1 },
        },
      },
    }],
  });
});
