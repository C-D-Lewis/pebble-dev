import { STORAGE_KEY_PLUGIN_STATS } from '../constants';
import { PluginStats } from '../types';

/**
 * Save the stats expected by the Pebble Plugin.
 */
export const savePluginStats = (stats: PluginStats) => {
  localStorage.setItem(STORAGE_KEY_PLUGIN_STATS, JSON.stringify(stats));
  console.log(`Saved plugin stats: ${JSON.stringify(stats)}`);
};
