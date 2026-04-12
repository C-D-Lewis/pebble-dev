import { Fabricate, FabricateComponent } from 'fabricate.js';
import { AppState, GlobalStatItem } from '../types.ts';

declare const fabricate: Fabricate<AppState>;

/**
 * TableHeader component.
 *
 * @param {object} props - Component props.
 * @param {string} props.textAlign - textAlign value to use.
 * @returns {FabricateComponent} TableHeader component.
 */
const TableHeader = ({ textAlign }: { textAlign: string }) => fabricate('th')
  .setStyles({
    color: 'white',
    fontSize: '0.9rem',
    fontWeight: 'bold',
    textAlign,
  });

/**
 * TableCell component.
 *
 * @param {object} props - Component props.
 * @param {string} props.textAlign - textAlign value to use.
 * @returns {FabricateComponent} TableCell component.
 */
const TableCell = ({ textAlign }: { textAlign: string }) => fabricate('td')
  .setStyles({
    color: '#ccc',
    fontSize: '0.9rem',
    textAlign,
    fontFamily: textAlign === 'left' ? 'monospace' : 'sans-serif',
  });

/**
 * GlobalStatsTable component.
 *
 * @param {object} props - Component props.
 * @param {string} props.field - Global stats list to use.
 * @returns {FabricateComponent} GlobalStatsTable component.
 */
export const GlobalStatsTable = (
  { field }: { field: 'models' | 'platforms' },
) => {
  const headerRow = fabricate('tr')
    .setChildren([
      TableHeader({ textAlign: 'left' }).setText('Name'),
      TableHeader({ textAlign: 'center' }).setText('#'),
      // TableHeader({ textAlign: 'center' }).setText('Avg. Battery'),
      TableHeader({ textAlign: 'center' }).setText('Median Battery'),
      TableHeader({ textAlign: 'center' }).setText('Avg. Rate'),
    ]);

  return fabricate('table')
    .setStyles({ marginBottom: '12px' })
    .setChildren([headerRow])
    .onCreate((el, state) => {
      const data = state.globalStats[field];

      el.addChildren(
        data.map((item: GlobalStatItem) => fabricate('tr')
          .setChildren([
            TableCell({ textAlign: 'left' }).setText(item.groupName),
            TableCell({ textAlign: 'center' }).setText(String(item.count)),
            // TableCell({ textAlign: 'center' }).setText(`${item.avgBatteryLife} days`),
            TableCell({ textAlign: 'center' }).setText(`${item.medianBatteryLife} days`),
            TableCell({ textAlign: 'center' }).setText(`${item.avgRate}%`),
          ])),
      );
    });
};
