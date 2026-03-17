import { Fabricate, FabricateComponent } from 'fabricate.js';
import Chart, { ChartConfiguration } from 'chart.js/auto';
import zoomPlugin from 'chartjs-plugin-zoom';
import { AppState, ChartMode, HistoryItem } from './types.ts';
import { fetchGlobalStats, fetchWatchHistory } from './api.ts';
import Theme from './theme.ts';
import {
  ONE_MONTH_AGO,
  ONE_WEEK_AGO,
  STATUS_EMPTY,
  UI_URL,
} from './constants.ts';

declare const fabricate: Fabricate<AppState>;

/**
 * Subtitle component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const Subtitle = () => fabricate('Text')
  .setStyles({
    color: 'white',
    fontSize: '1.2rem',
    marginTop: '5px',
    fontWeight: 'bold',
    textAlign: 'center',
    cursor: 'default',
  });

/**
 * Text component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const Text = () => fabricate('Text')
  .setStyles({ color: 'white', textAlign: 'center' });

/**
 * Annotation component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const Annotation = () => fabricate('Text')
  .setStyles(({ palette }) => ({
    color: palette.grey(9),
    textAlign: 'center',
    fontSize: '0.8rem',
    fontStyle: 'italic',
    marginTop: '8px',
  }));

/**
 * AppNavBar component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const AppNavBar = () => fabricate('Row')
  .setStyles(({ palette }) => ({
    padding: '2px 10px',
    backgroundColor: palette.primary,
    alignItems: 'center',
  }))
  .setChildren([
    fabricate('Image')
      .setAttributes({ src: 'assets/images/icon.png' })
      .setStyles({
        width: '48px',
        height: '48px',
        margin: '0px 10px',
        borderRadius: '50px',
      }),
    fabricate('Text')
      .setText('Muninn - Battery Wisdom')
      .setStyles({ color: 'white', fontSize: '1.2rem', cursor: 'default' }),
  ]);

/**
 * Footer component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const Footer = () => fabricate('Column')
  .setStyles({
    padding: '10px 0px',
    backgroundColor: '#000',
  })
  .setChildren([
    fabricate('Row')
      .setStyles({ justifyContent: 'center', padding: '5px' })
      .setChildren([
        fabricate('img')
          .setAttributes({ src: './assets/images/github.png' })
          .setStyles({
            width: '32px',
            height: '32px',
            cursor: 'pointer',
          })
          .onClick(
            () => window.open('https://github.com/C-D-Lewis/pebble-dev/tree/master/watchapps/muninn', '_blank'),
          ),
        fabricate('FabricateAttribution')
          .setStyles({ marginLeft: '15px', width: '50px' }),
      ]),
  ]);

/**
 * SearchBox component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const SearchBox = () => fabricate('Input')
  .setStyles(({ palette }) => ({
    padding: '8px',
    backgroundColor: palette.grey(2),
    color: 'white',
    border: 'none',
    borderRadius: '5px',
    fontSize: '1.1rem',
    width: '150px',
    textAlign: 'center',
    fontFamily: 'monospace',
  }))
  .setAttributes({ placeholder: '' })
  .onCreate((el, { id }) => {
    if (!id.length) return;

    const input = el as unknown as HTMLInputElement;
    input.value = id;
  })
  .onChange(async (el) => {
    const input = el as unknown as HTMLInputElement;

    const id = input.value.toUpperCase();
    input.value = id;
    fabricate.update({ id });
  });

/**
 * AppButton component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const AppButton = () => fabricate('Button')
  .setStyles(({ palette }) => ({
    padding: '9px',
    backgroundColor: palette.primary,
    color: 'white',
  }));

/**
 * SubmitButton component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const SubmitButton = () => AppButton()
  .setStyles({ marginLeft: '10px' })
  .setText('Search')
  .onClick(async (el, { id }) => {
    if (id.length !== 6) return;

    fetchWatchHistory(id);
  });

/**
 * Separator component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const Separator = () => fabricate('div')
  .setStyles(({ palette }) => ({
    backgroundColor: palette.grey(5),
    height: '2px',
    width: '90%',
    margin: '16px auto',
  }));

/**
 * AppLoader component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const AppLoader = () => fabricate('Loader', {
  size: 48,
  lineWidth: 5,
  color: Theme.palette.primary,
  backgroundColor: Theme.palette.grey(7),
})
  .setStyles({ margin: 'auto', marginTop: '15px' });

/**
 * InfoChip component.
 *
 * @param {object} props - Component props.
 * @param {string} props.label - Chip label.
 * @param {string} props.value - Chip value.
 * @returns {FabricateComponent} Fabricate component.
 */
const InfoChip = ({ label, value }: { label: string, value: string }) => fabricate('Row')
  .setStyles(({ palette }) => ({
    backgroundColor: palette.grey(4),
    borderRadius: '5px',
    margin: '3px',
    alignItems: 'center',
  }))
  .setChildren([
    fabricate('Text')
      .setStyles(({ palette }) => ({
        color: palette.grey(9),
        fontSize: '0.9rem',
      }))
      .setText(label),
    fabricate('Text')
      .setStyles({ color: 'white', fontSize: '0.9rem', fontFamily: 'monospace' })
      .setText(value),
  ]);

/**
 * StatView component.
 *
 * @param {object} props - Component props.
 * @param {string} props.label - Stat label.
 * @param {string} props.value - Stat value.
 * @returns {FabricateComponent} Fabricate component.
 */
const StatView = ({ label, value }: { label: string, value: string }) => fabricate('Column')
  .setStyles(({ palette }) => ({
    backgroundColor: palette.grey(4),
    borderRadius: '5px',
    padding: '8px 4px 12px 4px',
    margin: '3px',
    flex: '1',
  }))
  .setChildren([
    fabricate('Text')
      .setStyles(({ palette }) => ({
        color: palette.grey(9),
        fontSize: '0.9rem',
        textAlign: 'center',
      }))
      .setText(label),
    fabricate('Text')
      .setStyles({
        color: 'white',
        fontSize: '1.2rem',
        textAlign: 'center',
        margin: '0px',
        fontWeight: 'bold',
      })
      .setText(value),
  ]);

/**
 * StatsList component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const StatsList = () => fabricate('Column')
  .onCreate((el, state) => {
    const { stats } = state;
    const {
      count,
      totalDays,
      allTimeRate,
      lastWeekRate,
      numCharges,
      mtbc,
    } = stats;

    // Could be empty if not much data
    const lvarValue = lastWeekRate !== STATUS_EMPTY ? `${lastWeekRate}% per day` : '-';
    const mtbcValue = mtbc !== STATUS_EMPTY ? `${mtbc} days` : '-';

    const atelValue = Math.round(100 / allTimeRate);
    const lwelValue = lastWeekRate !== STATUS_EMPTY
      ? `${Math.round(100 / lastWeekRate)} days`
      : '-';

    el.setChildren([
      fabricate('Row')
        .setChildren([
          StatView({ label: 'Samples', value: String(count) }),
          StatView({ label: 'Duration', value: `${totalDays} days` }),
        ]),
      fabricate('Row')
        .setChildren([
          StatView({ label: 'Avg. Discharge Rate', value: `${allTimeRate}% per day` }),
          StatView({ label: 'Last Week Avg. Rate', value: lvarValue }),
        ]),
      fabricate('Row')
        .setChildren([
          StatView({ label: 'Charge Events', value: `${numCharges} events` }),
          StatView({ label: 'Avg. Charge Interval', value: mtbcValue }),
        ]),
      fabricate('Row')
        .setChildren([
          StatView({ label: 'Est. Battery Life', value: `${atelValue} days` }),
          StatView({ label: 'Last Week Est. Life', value: lwelValue }),
        ]),
    ]);
  });

/**
 * InfoChips component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const InfoChips = () => fabricate('Column')
  .onCreate((el, state) => {
    const {
      id,
      model,
      firmware,
      platform,
    } = state;

    el.setChildren([
      fabricate('Row')
        .setStyles({ flexWrap: 'wrap' })
        .setChildren([
          InfoChip({ label: 'Model', value: model }),
          InfoChip({ label: 'Firmware', value: firmware }),
          InfoChip({ label: 'Platform', value: platform }),
          InfoChip({ label: 'Muninn ID', value: id }),
        ]),
    ]);
  });

/**
 * HistoryChart component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const HistoryChart = () => {
  // TODO: Use to filter data on this week / last two charges / all time
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
      const canvas = fabricate('canvas');
      el.setChildren([canvas]);

      // @ts-expect-error - Chart.js types problem
      Chart.register(zoomPlugin);

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

/**
 * GlobalStatsList component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const GlobalStatsList = () => fabricate('Column')
  .onCreate((el) => {
    el.setChildren([
      AppLoader(),
    ]);

    fetchGlobalStats();
  })
  .onUpdate(async (el, state) => {
    const { globalStats } = state;
    const { totalUploads } = globalStats;

    el.setChildren([
      fabricate('Row')
        .setChildren([
          StatView({ label: 'User Uploads', value: `${totalUploads}` }),
        ]),
    ]);
  }, ['globalStats']);

/**
 * ShareLink component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const ShareLink = () => fabricate('Text')
  .setStyles(({ palette }) => ({
    color: 'white',
    textAlign: 'center',
    padding: '8px',
    margin: '5px auto',
    backgroundColor: palette.grey(2),
    borderRadius: '5px',
    fontFamily: 'monospace',
    fontSize: '0.85rem',
  }))
  .onCreate((el, { id }) => {
    const shareUrl = `${UI_URL}?id=${id}`;

    el.setAttributes({ href: shareUrl, target: '_blank' });
    el.setText(shareUrl);
  });

/**
 * ChartModeButton component.
 *
 * @param {object} props - Component props.
 * @param {string} props.label - Button label.
 * @param {ChartMode} props.mode - Mode to set when pressed.
 * @returns {FabricateComponent} Fabricate component.
 */
const ChartModeButton = ({ label, mode }: { label: string, mode: ChartMode }) => AppButton()
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
const ChartModeBar = () => fabricate('Row')
  .setStyles({ justifyContent: 'center', marginTop: '10px' })
  .setChildren([
    ChartModeButton({ label: 'All', mode: 'all' }),
    ChartModeButton({ label: 'Last Week', mode: 'lastWeek' }),
    ChartModeButton({ label: 'Last Month', mode: 'lastMonth' }),
  ]);

/**
 * AppCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const AppCard = () => fabricate('Card')
  .setStyles(({ palette }) => ({
    padding: '16px',
    backgroundColor: palette.grey(3),
    boxShadow: 'none',
    margin: '15px auto',
    width: 'auto',
    transition: '0.5s',
    opacity: '0',
  }))
  .onCreate((el) => {
    // Fade in
    setTimeout(() => el.setStyles({ opacity: '1' }), 100);
  });

/**
 * LoginCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const LoginCard = () => AppCard()
  .setStyles({ marginTop: '28px' })
  .setChildren([
    Subtitle()
      .setText('Welcome!'),
    Text().setText('Please enter the six digit code shown on your watch:'),
    fabricate('Row')
      .setStyles({ marginTop: '10px', alignItems: 'center', justifyContent: 'center' })
      .setChildren([
        SearchBox(),
        SubmitButton(),
      ]),
  ]);

/**
 * HistoryCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const HistoryCard = () => AppCard()
  .setChildren([
    Subtitle().setText('Your History'),
    Text().setText('Below is the complete battery history as uploaded from Muninn. It will be updated each time you share from the watchapp.'),
    Text()
      .setStyles(({ palette }) => ({ color: palette.grey(9) }))
      .onCreate((el, { updatedAt }) => {
        if (!updatedAt) return;

        const date = new Date(updatedAt);
        el.setText(`Last updated: ${date.toLocaleString()}`);
      }),
    Separator(),
    Subtitle().setText('All-time Graph'),
    HistoryChart(),
    ChartModeBar(),
    Annotation().setText('Try zooming and panning to see more detail.'),
    Separator(),
    Subtitle().setText('All-time Stats'),
    StatsList(),
    Separator(),
    Subtitle().setText('Watch Info'),
    InfoChips(),
    Separator(),
    Subtitle().setText('Share'),
    Text().setText('Copy the link below to share your battery stats:'),
    ShareLink(),
    Separator(),
    Annotation().setText('That\'s all we know - thanks for using Muninn!'),
  ]);

/**
 * GlobalStatsCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const GlobalStatsCard = () => AppCard()
  .setStyles({ marginTop: '25px' })
  .setChildren([
    Subtitle().setText('Global Stats'),
    GlobalStatsList(),
  ]);

/**
 * NotFoundCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const NotFoundCard = () => AppCard()
  .setStyles({ marginTop: '25px' })
  .setChildren([
    fabricate('Image', { src: 'assets/images/not-found.png' })
      .setStyles({ width: '96px', height: '96px', margin: 'auto' }),
    Subtitle().setText('Not Found'),
    Text()
      .setStyles({ maxWidth: '350px' })
      .setText('No data was found for this code.\n\nUse the upload option in the watchapp to start seeing data here.'),
    Annotation().setText('(not available for original Pebble due to memory constraints)'),
  ]);

/**
 * Braid component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const Braid = () => fabricate('div')
  .setStyles({
    height: '18px',
    width: 'auto',
    backgroundImage: "url('assets/images/braid.png')",
    backgroundRepeat: 'repeat-x',
    backgroundSize: 'contain',
  });
