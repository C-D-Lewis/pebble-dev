import { Fabricate } from 'fabricate.js';
import { AppState } from './types.ts';
import { API_URL } from './constants.ts';

declare const fabricate: Fabricate<AppState>;

/**
 * Fetch the data for this watch ID.
 *
 * @param {string} id Watch ID to fetch data for, issued by API.
 * @returns {Promise<void>}
 */
export const fetchWatchHistory = async (id: string) => {
  fabricate.update({ loading: true });

  try {
    const res = await fetch(`${API_URL}/history/${id}`);
    if (!res.ok) {
      throw new Error(`Failed to fetch data: ${res.statusText}`);
    }

    const data = await res.json();
    fabricate.update({
      id,
      history: data.history.reverse(),
      platform: data.platform,
      model: data.model,
      firmware: data.firmware,
      stats: data.stats,
    });
  } catch (err) {
    fabricate.update({ notFound: true });
  } finally {
    fabricate.update({ loading: false });
  }
};

/**
 * Fetch the stats endpoint for general stats.
 *
 * @returns {Promise<void>}
 */
export const fetchGlobalStats = async () => {
  const res = await fetch(`${API_URL}/globalStats`);
  if (!res.ok) throw new Error(`Failed to fetch data: ${res.statusText}`);

  const data = await res.json();
  fabricate.update({ globalStats: data });
};
