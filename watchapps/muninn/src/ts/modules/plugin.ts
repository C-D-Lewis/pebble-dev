import { STORAGE_KEY_PLUGIN_STATS } from '../constants';
import { PluginStats } from '../types';

/**
 * Save the stats expected by the Pebble plugin.
 */
export const savePluginStats = (stats: PluginStats) => {
  localStorage.setItem(STORAGE_KEY_PLUGIN_STATS, JSON.stringify(stats));
  console.log(`Plugin stats: ${JSON.stringify(stats)}`);
};
