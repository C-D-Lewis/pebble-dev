import { Fabricate, FabricateComponent } from 'fabricate.js';
import Chart, { ChartConfiguration } from 'chart.js/auto';
import zoomPlugin from 'chartjs-plugin-zoom';
import { AppState } from './types.ts';
import { fetchGlobalStats, fetchWatchHistory } from './api.ts';
import Theme from './theme.ts';

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
    padding: '10px',
    backgroundColor: palette.secondary,
    alignItems: 'center',
  }))
  .setChildren([
    fabricate('Image')
      .setAttributes({ src: 'assets/images/icon.png' })
      .setStyles({
        width: '40px',
        height: '40px',
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

    fabricate.update({ id: input.value });
  });

/**
 * SubmitButton component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const SubmitButton = () => fabricate('Button')
  .setStyles(({ palette }) => ({
    marginLeft: '10px',
    padding: '9px 4px',
    backgroundColor: palette.primary,
    color: 'white',
  }))
  .setText('Search')
  .onClick(async (el, { id }) => {
    if (id.length !== 6) return;

    fabricate.update({ loading: true });
    try {
      await fetchWatchHistory(id);
    } catch (err) {
      alert('Failed to load data. Please check the code and try again.');
    } finally {
      fabricate.update({ loading: false });
    }
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
    padding: '4px',
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
    padding: '4px',
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
        fontSize: '1.1rem',
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

    el.setChildren([
      fabricate('Row')
        .setChildren([
          StatView({ label: 'Samples', value: String(count) }),
          StatView({ label: 'Duration', value: `${totalDays} days` }),
        ]),
      fabricate('Row')
        .setChildren([
          StatView({ label: 'All-Time Avg. Rate', value: `${allTimeRate}% per day` }),
          StatView({ label: 'Last Week Avg. Rate', value: `${lastWeekRate}% per day` }),
        ]),
      fabricate('Row')
        .setChildren([
          StatView({ label: 'Charge Events', value: `${numCharges} events` }),
          StatView({ label: 'Avg. Charge Interval', value: `${mtbc} days` }),
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
    const { model, firmware, platform } = state;

    el.setChildren([
      fabricate('Row')
        .setChildren([
          InfoChip({ label: 'Model', value: model }),
        ]),
      fabricate('Row')
        .setChildren([
          InfoChip({ label: 'Firmware', value: firmware }),
          InfoChip({ label: 'Platform', value: platform }),
        ]),
    ]);
  });

/**
 * HistoryChart component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const HistoryChart = () => fabricate('div')
  .setStyles(({ palette }) => ({
    maxWidth: '100%',
    maxHeight: '500px',
    borderRadius: '5px',
    backgroundColor: palette.grey(2),
    padding: '4px',
  }))
  .onCreate((el, state) => {
    const canvas = fabricate('canvas')
      .setStyles({ width: '100%', height: '100%' });
    el.setChildren([canvas]);

    // @ts-expect-error - Chart.js types problem
    Chart.register(zoomPlugin);

    // Using chart.js
    const opts: ChartConfiguration = {
      type: 'line',
      data: {
        labels: state.history.map((entry) => {
          const d = new Date(entry.ts * 1000);
          return d.toLocaleString('en-GB', { day: 'numeric', month: 'short' });
        }),
        datasets: [
          {
            label: 'Battery Level',
            data: state.history.map((entry) => entry.cp),
            borderColor: Theme.palette.primary,
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
    new Chart(canvas as unknown as HTMLCanvasElement, opts);
  });

/**
 * GlobalStatsList component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const GlobalStatsList = () => fabricate('Column')
  .onCreate((el) => {
    el.setChildren([
      fabricate('Loader'),
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
 * AppCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const AppCard = () => fabricate('Card')
  .setStyles(({ palette }) => ({
    padding: '8px',
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
    fabricate('Text')
      .setStyles({ color: 'white', textAlign: 'center' })
      .setText('Please enter the six digit code shown on your watch:'),
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
    fabricate('Text')
      .setStyles({ color: 'white', textAlign: 'center' })
      .setText('Below is the complete battery history as uploaded from Muninn.'),
    Separator(),
    Subtitle().setText('All-time Graph'),
    HistoryChart(),
    Annotation().setText('Try zooming and panning to see more detail.'),
    Separator(),
    Subtitle().setText('All-time Stats'),
    StatsList(),
    Separator(),
    Subtitle().setText('Watch Info'),
    InfoChips(),
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
 * Braid component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const Braid = () => fabricate('div')
  .setStyles({
    height: '16px',
    width: 'auto',
    backgroundImage: "url('assets/images/braid.png')",
    backgroundRepeat: 'repeat-x',
    backgroundSize: 'contain',
  });
