import { Fabricate, FabricateComponent } from 'fabricate.js';
import { ChartModeBar, HistoryChart } from './chart.ts';
import { AppState } from '../types.ts';
import {
  CardTitle,
  SearchBox,
  SubmitButton,
  Annotation,
  Text,
  Separator,
  StatsList,
  ShareLink,
  GlobalStatsView,
  AppButton,
  AppLoader,
  InfoChips,
} from './index.ts';
import { downloadChartImage, downloadHistoryCsv, getParam } from '../util.ts';
import { fetchGlobalStats } from '../api.ts';

declare const fabricate: Fabricate<AppState>;

/**
 * AppCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const AppCard = () => fabricate('Card')
  .setStyles(({ palette }) => ({
    padding: '8px 14px',
    backgroundColor: palette.grey(3),
    boxShadow: 'none',
    margin: '5px auto 10px auto',
    width: '90%',
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
    CardTitle()
      .setText('Welcome!'),
    Text().setText('Please enter the six digit code shown on your watch to see your full history:'),
    fabricate('Row')
      .setStyles({ marginTop: '10px', alignItems: 'center', justifyContent: 'center' })
      .setChildren([
        SearchBox(),
        SubmitButton(),
      ]),
    Separator(),
    Annotation().setText('(not available for original Pebble due to memory constraints)'),
  ]);

/**
 * SummaryCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const SummaryCard = () => AppCard()
  .setChildren([
    CardTitle().setText('Your History'),
    Text()
      .setText('Below is the complete battery history as uploaded from Muninn. It will be updated each time you share from the watchapp.'),
    Annotation().onCreate((el, { updatedAt }) => {
      if (!updatedAt) return;

      const date = new Date(updatedAt);
      el.setText(`Last updated: ${date.toLocaleString()}`);
    }),
  ]);

/**
 * ChartCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const ChartCard = () => AppCard()
  .setStyles({ position: 'relative' })
  .setChildren([
    CardTitle().setText('Graph'),
    HistoryChart(),
    ChartModeBar(),
    Separator(),
    Annotation().setText('Try zooming and panning to see more detail.'),
    // DownloadGraphButton(),  // Can't open this in config page view...
  ]);

/**
 * StatsCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const StatsCard = () => AppCard()
  .setChildren([
    CardTitle().setText('Statistics'),
    StatsList(),
    Separator(),
    Annotation().setText('If some stats aren\'t ready yet, check back in a few more days.'),
  ]);

/**
 * ShareCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const ShareCard = () => {
  const card = AppCard()
    .setChildren([
      CardTitle().setText('Share'),
      Text().setText('Copy the link below to share your battery stats:'),
      ShareLink(),
      Separator(),
    ]);

  // Coreapp config page webview doesn't allow copying or opening save dialogs apparently
  if (!getParam('isAppConfigPage')) {
    card.addChildren([
      CardTitle().setText('Export Data'),
      fabricate('Row')
        .setStyles({ justifyContent: 'center' })
        .setChildren([
          AppButton()
            .setStyles({ padding: '6px', fontSize: '0.9rem' })
            .setText('Graph (PNG)')
            .onClick(downloadChartImage),
          AppButton()
            .setStyles({ padding: '6px', fontSize: '0.9rem' })
            .setText('History (CSV)')
            .onClick((el, { history }) => downloadHistoryCsv(history)),
        ]),
    ]);
  } else {
    card.addChildren([
      Annotation().setText('View in a full browser for more export options.'),
    ]);
  }

  return card;
};

/**
 * HistoryCardList component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const HistoryCardList = () => fabricate('Column')
  .setChildren([
    SummaryCard(),
    ChartCard(),
    StatsCard(),
    ShareCard(),
    InfoChips(),
    Text()
      .setStyles({ marginTop: '16px' })
      .setText('That\'s all we know - thanks for using Muninn!'),
  ]);

/**
 * GlobalStatsCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const GlobalStatsCard = () => AppCard()
  .setStyles({ marginTop: '25px' })
  .setChildren([
    CardTitle().setText('Global Stats'),
    Text().setText('These averages are derived from all Muninn users who opted to upload their history. Over time, more users will hopefully increase data accuracy.'),
    Separator(),
    fabricate.conditional(
      (state) => state.globalStats.historyCount !== 0,
      GlobalStatsView,
    ),
    fabricate.conditional(
      (state) => state.globalStats.historyCount === 0,
      AppLoader,
    ),
  ])
  .onCreate((el, { globalStats }) => {
    if (globalStats.historyCount !== 0) return;

    fetchGlobalStats();
  });

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
    CardTitle().setText('Not Found'),
    Text()
      .setStyles({ maxWidth: '350px', margin: '5px auto' })
      .setText('No data was found for this code. Use the upload option in the watchapp to start seeing data here.'),
    Separator(),
    Annotation().setText('Not available for Pebble classic/Pebble Steel due to memory constraints. Sorry!'),
  ]);
