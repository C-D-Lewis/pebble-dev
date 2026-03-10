import { Fabricate, FabricateComponent } from 'fabricate.js';
import {
  AppNavBar, Footer, HistoryCard, LoginCard, Braid,
} from './components.ts';
import { AppState } from './types.ts';
import Theme from './theme.ts';

declare const fabricate: Fabricate<AppState>;

/**
 * AppContent component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const AppContent = () => fabricate('Column')
  .setStyles({
    padding: '8px',
    overflowY: 'scroll',
    minHeight: '90vh',
    maxWidth: '550px',
    margin: 'auto',
  })
  .setChildren([
    fabricate.conditional(
      (state) => !state.loading && state.history.length === 0,
      LoginCard,
    ),
    // Save this until it's more useful
    // fabricate.conditional(
    //   (state) => !state.loading && state.history.length === 0,
    //   GlobalStatsCard,
    // ),
    fabricate.conditional(
      (state) => state.loading && state.history.length === 0,
      () => fabricate('Loader', {
        size: 48,
        lineWidth: 5,
        color: Theme.palette.primary,
        backgroundColor: Theme.palette.grey(4),
      })
        .setStyles({ margin: 'auto', marginTop: '15px' }),
    ),
    fabricate.conditional(
      (state) => !state.loading && state.history.length > 0,
      HistoryCard,
    ),
  ]);

/**
 * App component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const App = () => fabricate('Column')
  .setStyles({
    margin: fabricate.isNarrow() ? '0px' : '0px auto',
  })
  .setChildren([
    AppNavBar(),
    Braid(),
    AppContent(),
    Braid(),
    Footer(),
  ]);

// If in the path, use it
const path = new URL(window.location.href).pathname;
const id = path.length > 1 ? path.slice(1) : '';

const initialState: AppState = {
  loading: false,
  id,
  history: [],
  platform: '',
  model: '',
  firmware: '',
  stats: {
    count: 0,
    totalDays: 0,
    allTimeRate: 0,
    lastWeekRate: 0,
    numCharges: 0,
    mtbc: 0,
  },
  globalStats: {
    totalUploads: 0,
  },
};

fabricate.app(App, initialState, { theme: Theme });
