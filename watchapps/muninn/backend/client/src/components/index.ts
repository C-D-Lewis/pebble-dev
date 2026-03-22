import { Fabricate, FabricateComponent } from 'fabricate.js';
import { AppState } from '../types.ts';
import { fetchGlobalStats, fetchWatchHistory } from '../api.ts';
import Theme from '../theme.ts';
import {
  STATUS_EMPTY,
  UI_URL,
} from '../constants.ts';

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
  .setStyles({
    color: 'white',
    textAlign: 'center',
    fontSize: '0.9rem',
  });

/**
 * ImageButton component.
 *
 * @param {object} props - Component props.
 * @param {string} props.src - Image source.
 * @returns {FabricateComponent} Fabricate component.
 */
export const ImageButton = ({ src }: { src: string }) => fabricate('Image', { src })
  .setStyles(({ palette }) => ({
    width: '24px',
    height: '24px',
    backgroundColor: palette.grey(3),
    borderRadius: '5px',
    padding: '4px',
  }));

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
export const AppButton = () => fabricate('Button')
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
export const Separator = () => fabricate('div')
  .setStyles(({ palette }) => ({
    backgroundColor: palette.grey(5),
    height: '2px',
    width: '90%',
    margin: '12px auto',
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
    margin: '8px',
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
export const StatsList = () => fabricate('Column')
  .setStyles({ marginTop: '4px 8px' })
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
export const InfoChips = () => fabricate('Column')
  .setStyles({ marginTop: '8px' })
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
 * GlobalStatsList component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const GlobalStatsList = () => fabricate('Column')
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
export const ShareLink = () => fabricate('Text')
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
