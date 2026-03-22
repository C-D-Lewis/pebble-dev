import { Fabricate, FabricateComponent } from 'fabricate.js';
import { ChartModeBar, HistoryChart } from './chart.ts';
import { AppState } from '../types.ts';
import {
  Subtitle,
  SearchBox,
  SubmitButton,
  Annotation,
  Text,
  Separator,
  StatsList,
  InfoChips,
  ShareLink,
  GlobalStatsList,
  ImageButton,
} from './index.ts';
import { downloadChartImage } from '../util.ts';

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
 * SummaryCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const SummaryCard = () => AppCard()
  .setChildren([
    Subtitle().setText('Your History'),
    Text()
      .setText('Below is the complete battery history as uploaded from Muninn. It will be updated each time you share from the watchapp.'),
    Text()
      .setStyles(({ palette }) => ({ color: palette.grey(9) }))
      .onCreate((el, { updatedAt }) => {
        if (!updatedAt) return;

        const date = new Date(updatedAt);
        el.setText(`Last updated: ${date.toLocaleString()}`);
      }),
  ]);

/**
 * DownloadGraphButton component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const DownloadGraphButton = () => ImageButton({ src: 'assets/images/download.png' })
  .setStyles({
    position: 'absolute',
    top: '10px',
    right: '16px',
  })
  .onClick(downloadChartImage);

/**
 * ChartCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const ChartCard = () => AppCard()
  .setStyles({ position: 'relative' })
  .setChildren([
    Subtitle().setText('Graph'),
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
    Subtitle().setText('Statistics'),
    StatsList(),
    Separator(),
    Annotation().setText('If some stats aren\'t ready yet, check back in a few more days.'),
  ]);

/**
 * InfoCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const InfoCard = () => AppCard()
  .setChildren([
    Subtitle().setText('Watch Data'),
    Text().setText('Simple metadata about this Pebble watch:'),
    InfoChips(),
    Separator(),
    Annotation().setText('Hopefully soon it will be possible to compare firmwares and models to see even more insight into Pebble watch performance.'),
  ]);

/**
 * ShareCard component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const ShareCard = () => AppCard()
  .setChildren([
    Subtitle().setText('Share'),
    Text().setText('Copy the link below to share your battery stats:'),
    ShareLink(),
    Separator(),
    Annotation().setText('That\'s all we know - thanks for using Muninn!'),
  ]);

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
    InfoCard(),
    ShareCard(),
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
