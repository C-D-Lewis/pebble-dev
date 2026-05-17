import { Fabricate, FabricateComponent } from 'fabricate.js';
import { AppState } from '../types.ts';
import { fetchWatchHistory } from '../api.ts';
import Theme from '../theme.ts';
import { UI_URL } from '../constants.ts';
import { GlobalStatsTable } from './table.ts';

declare const fabricate: Fabricate<AppState>;

/**
 * CardTitle component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const CardTitle = () => fabricate('Text')
  .setStyles({
    color: 'white',
    fontSize: '1.2rem',
    marginTop: '5px',
    fontWeight: 'bold',
    textAlign: 'center',
    cursor: 'default',
  });

/**
 * Subtitle component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const Subtitle = () => fabricate('Text')
  .setStyles({
    color: 'white',
    fontSize: '1rem',
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
    marginTop: '4px',
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
        width: '36px',
        height: '36px',
        margin: '0px 10px',
        padding: '8px',
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
 * GlobalStatsView component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const GlobalStatsView = () => fabricate('Column')
  .onUpdate(async (el, state) => {
    const { globalStats } = state;
    const { historyCount, updatedAt  } = globalStats;

    const date = new Date(updatedAt);

    el.setChildren([
      fabricate('Column')
        .setChildren([
          Subtitle().setText('By Model Name'),
          GlobalStatsTable({ field: 'models' }),
          Separator(),
          Subtitle().setText('By Platform'),
          GlobalStatsTable({ field: 'platforms' }),
          Separator(),
          Annotation().setText(`From ${historyCount} users, last updated: ${date.toLocaleString()}`),
        ]),
    ]);
  }, [fabricate.StateKeys.Created, 'globalStats']);

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
