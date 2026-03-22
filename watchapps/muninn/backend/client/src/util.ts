import { CANVAS_ID } from './constants.ts';

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
