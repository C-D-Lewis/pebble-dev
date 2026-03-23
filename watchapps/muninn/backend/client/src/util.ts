import { CANVAS_ID } from './constants.ts';
import { HistoryItem } from './types.ts';

const params = new URLSearchParams(window.location.search);

/**
 * Trigger download of the chart as an image.
 */
export const downloadChartImage = () => {
  const canvas = document.getElementById(CANVAS_ID) as unknown as HTMLCanvasElement;
  const imageURI = canvas.toDataURL('image/png');

  const a = document.createElement('a');
  a.download = 'muninn-chart.png';
  a.href = imageURI;

  a.click();
};

/**
 * Trigger download of history as CSV. Currently a subset of what's truly stored on the JS side.
 *
 * @param {HistoryItem[]} history - Watch sample history.
 */
export const downloadHistoryCsv = (history: HistoryItem[]) => {
  const headers = 'timestamp,charge_percent,rate,result\n';
  const rows = history.map((item) => `${item.ts},${item.cp},${item.r},${item.res}`).join('\n');
  const csvContent = headers + rows;

  const blob = new Blob([csvContent], { type: 'text/csv' });
  const url = URL.createObjectURL(blob);

  const a = document.createElement('a');
  a.download = 'muninn-history.csv';
  a.href = url;

  a.click();
};

/**
 * Get a query param.
 *
 * @param {string} name - Param name.
 * @returns {string} Param if found.
 */
export const getParam = (name: string) => params.get(name);
