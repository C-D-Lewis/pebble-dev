import { Fabricate, FabricateComponent } from 'fabricate.js';
import Chart, { ChartConfiguration } from 'chart.js/auto';
import zoomPlugin from 'chartjs-plugin-zoom';
import { AppState, ChartMode, HistoryItem } from '../types.ts';
import { AppButton } from './index.ts';
import { ONE_WEEK_AGO, ONE_MONTH_AGO, CANVAS_ID } from '../constants.ts';
import Theme from '../theme.ts';

declare const fabricate: Fabricate<AppState>;

/**
 * ChartModeButton component.
 *
 * @param {object} props - Component props.
 * @param {string} props.label - Button label.
 * @param {ChartMode} props.mode - Mode to set when pressed.
 * @returns {FabricateComponent} Fabricate component.
 */
export const ChartModeButton = ({ label, mode }: { label: string, mode: ChartMode }) => AppButton()
  .setStyles({ padding: '4px', fontSize: '0.8rem' })
  .setText(label)
  .onClick(() => fabricate.update({ chartMode: mode }))
  .onUpdate((el, { chartMode }) => {
    const isMe = mode === chartMode;

    el.setStyles(({ palette }) => ({
      backgroundColor: isMe ? palette.primary : palette.grey(4),
      fontWeight: isMe ? 'bold' : 'initial',
    }));
  }, [fabricate.StateKeys.Created, 'chartMode']);

/**
 * ChartModeBar component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const ChartModeBar = () => fabricate('Row')
  .setStyles({ justifyContent: 'center', marginTop: '10px' })
  .setChildren([
    ChartModeButton({ label: 'All', mode: 'all' }),
    ChartModeButton({ label: 'Last Week', mode: 'lastWeek' }),
    ChartModeButton({ label: 'Last Month', mode: 'lastMonth' }),
  ]);

/**
 * HistoryChart component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const HistoryChart = () => {
  let chartRef: Chart;

  /**
   * Filter a HistoryItem using current mode.
   *
   * @param {HistoryItem} item - Item to filter.
   * @param {ChartMode} mode - Current chart mode.
   * @returns {boolean} true if should be included.
   */
  const filterHistoryItem = (item: HistoryItem, mode: ChartMode) => {
    const date = new Date(item.ts * 1000);
    if (mode === 'lastWeek') return date.getTime() >= ONE_WEEK_AGO;
    if (mode === 'lastMonth') return date.getTime() >= ONE_MONTH_AGO;
    return true;
  };

  /**
   * Create labels with a mode and history to filter.
   *
   * @param {HistoryItem[]} history - History of samples to use.
   * @param {ChartMode} mode - Chart mode to use.
   * @returns {string[]} Labels to use.
   */
  const createLabels = (history: HistoryItem[], mode: ChartMode) => history
    .filter((p) => filterHistoryItem(p, mode))
    .map((item) => {
      const d = new Date(item.ts * 1000);
      return d.toLocaleString('en-GB', { day: 'numeric', month: 'short' });
    });

  /**
   * Create history datapoints with a mode and history to filter.
   *
   * @param {HistoryItem[]} history - History of samples to use.
   * @param {ChartMode} mode - Chart mode to use.
   * @returns {string[]} Labels to use.
   */
  const createHistoryData = (history: HistoryItem[], mode: ChartMode) => history
    .filter((p) => filterHistoryItem(p, mode))
    .map((p) => p.cp);

  return fabricate('div')
    .setStyles(({ palette }) => ({
      maxWidth: '100%',
      height: '230px',
      borderRadius: '5px',
      backgroundColor: palette.grey(2),
      padding: '4px',
      marginTop: '8px',
    }))
    .onCreate((el, { history, chartMode }) => {
      const canvas = fabricate('canvas').setAttributes({ id: CANVAS_ID });
      el.setChildren([canvas]);

      // @ts-expect-error - Chart.js types problem
      Chart.register(zoomPlugin);

      const backgroundColorPlugin = {
        id: 'drawBackgroundColor',
        /**
         * beforeDraw option.
         *
         * @param {object} chart - chart object
         */
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        beforeDraw: (chart: Chart) => {
          const { ctx } = chart;
          ctx.save();
          ctx.globalCompositeOperation = 'destination-over';
          ctx.fillStyle = Theme.palette.grey(2);
          ctx.fillRect(0, 0, chart.width, chart.height);
          ctx.restore();
        },
      };

      // Using chart.js
      const opts: ChartConfiguration = {
        type: 'line',
        data: {
          labels: createLabels(history, chartMode),
          datasets: [
            {
              label: 'Battery Level',
              data: createHistoryData(history, chartMode),
              borderColor: Theme.palette.secondary,
              backgroundColor: 'transparent',
              tension: 0.1,
              clip: false,
            },
          ],
        },
        plugins: [backgroundColorPlugin],
        options: {
          responsive: true,
          maintainAspectRatio: false,
          scales: {
            x: {
              ticks: {
                maxTicksLimit: 10,
              },
            },
            y: {
              beginAtZero: true,
              max: 100,
              grace: '5%',
            },
          },
          plugins: {
            legend: {
              display: false,
              position: 'bottom',
              align: 'center',
              labels: {
                color: Theme.palette.primary,
                font: {
                  size: 14,
                },
                usePointStyle: true,
                pointStyle: 'rounded',
              },
            },
            zoom: {
              zoom: {
                wheel: { enabled: true },
                pinch: { enabled: true },
                mode: 'x',
              },
              pan: { enabled: true, mode: 'x' },
            },
          },
        },
      };
      // eslint-disable-next-line no-new
      chartRef = new Chart(canvas as unknown as HTMLCanvasElement, opts);
    })
    .onUpdate((el, { history, chartMode }) => {
      chartRef.data.labels = createLabels(history, chartMode);
      chartRef.data.datasets[0].data = createHistoryData(history, chartMode);

      chartRef.update();
    }, ['chartMode']);
};
