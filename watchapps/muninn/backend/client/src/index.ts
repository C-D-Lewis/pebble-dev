import { Fabricate, FabricateComponent } from 'fabricate.js';
import { AppState } from './types.ts';
import Theme from './theme.ts';
import { fetchWatchHistory } from './api.ts';
import {
  LoginCard,
  HistoryCardList,
  NotFoundCard,
} from './components/cards.ts';
import {
  AppLoader,
  AppNavBar,
  Braid,
  Footer,
} from './components/index.ts';
import { getParam } from './util.ts';

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
    // TODO: Most of these are pages, use fabricate router
    fabricate.conditional(
      (state) => !state.loading && state.history.length === 0 && !state.notFound,
      LoginCard,
    ),
    fabricate.conditional(
      (state) => state.loading && state.history.length === 0,
      AppLoader,
    ),
    fabricate.conditional(
      (state) => !state.loading && state.history.length > 0,
      HistoryCardList,
    ),
    // Keep disabled until meaningful and large gaps are handled
    // fabricate.conditional(
    //   (state) => !state.loading && !state.notFound,
    //   GlobalStatsCard,
    // ),
    fabricate.conditional(
      (state) => !state.loading && state.notFound,
      NotFoundCard,
    ),
  ]);

/**
 * App component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
const App = () => fabricate('Column')
  .setStyles({ margin: fabricate.isNarrow() ? '0px' : '0px auto' })
  .setChildren([
    AppNavBar(),
    Braid(),
    AppContent(),
    Braid(),
    Footer(),
  ])
  .onCreate(() => {
    const id = getParam('id');
    if (!id) return;

    fetchWatchHistory(id.toUpperCase());
  });

const initialState: AppState = {
  loading: false,
  notFound: false,
  chartMode: 'all',

  id: '',
  updatedAt: 0,
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
    historyCount: 0,
    models: [],
    platforms: [],
    updatedAt: 0,
  },
};

fabricate.app(App, initialState, { theme: Theme });
