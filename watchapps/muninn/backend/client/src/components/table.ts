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
const TableHeader = ({ textAlign }: { textAlign: string }) => fabricate('div')
  .setStyles({
    color: 'white',
    fontSize: '0.9rem',
    fontWeight: 'bold',
    textAlign,
    flex: textAlign === 'left' ? '2 1 40%' : '1 1 20%',
    minWidth: '0',
  });

/**
 * TableCell component.
 *
 * @param {object} props - Component props.
 * @param {string} props.textAlign - textAlign value to use.
 * @param {boolean} props.muted - Whether to use muted styles.
 * @returns {FabricateComponent} TableCell component.
 */
const TableCell = ({ textAlign, muted }: { textAlign: string, muted: boolean }) => fabricate('div')
  .setStyles(({ palette }) => ({
    color: muted ? palette.grey(9) : 'white',
    fontSize: '0.8rem',
    textAlign,
    fontFamily: textAlign === 'left' ? 'monospace' : 'sans-serif',
    flex: textAlign === 'left' ? '2 1 40%' : '1 1 20%',
    minWidth: '0',
  }));

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
  const headerRow = fabricate('div')
    .setStyles({
      display: 'flex',
      alignItems: 'center',
      padding: '8px 0',
      borderBottom: '1px solid rgba(255,255,255,0.1)',
    })
    .setChildren([
      TableHeader({ textAlign: 'left' }).setText('Name'),
      TableHeader({ textAlign: 'center' }).setText('#'),
      TableHeader({ textAlign: 'center' }).setText('Battery'),
      TableHeader({ textAlign: 'center' }).setText('Rate'),
    ]);

  return fabricate('Column')
    .setStyles({
      marginBottom: '12px',
      display: 'flex',
      gap: '2px',
    })
    .setChildren([headerRow])
    .onCreate((el, state) => {
      const data = state.globalStats[field];

      el.addChildren(
        data.map((item: GlobalStatItem, i: number) => {
          const {
            count,
            groupName,
            avgBatteryLife,
            avgRate,
            // values,
          } = item;
          const muted = count < 5;
          // const maxValue = values.reduce((max: number, val: number) => Math.max(max, val), 0);

          const groupRow = fabricate('Row')
            .setStyles({ alignItems: 'center', padding: '4px 0' })
            .setChildren([
              TableCell({ textAlign: 'left', muted }).setText(groupName),
              TableCell({ textAlign: 'center', muted }).setText(String(count)),
              TableCell({ textAlign: 'center', muted }).setText(`${avgBatteryLife} days`),
              TableCell({ textAlign: 'center', muted }).setText(`${avgRate}%`),
            ]);

          // const heatmap = fabricate('div')
          //   .setStyles({ display: 'flex', margin: '4px 0 8px 0' })
          //   .setChildren(
          //     values.map((val: number) => {
          //       // Calculate deviation from average (-1.0 to 1.0 range)
          //       const deviation = (val - avgBatteryLife) / maxValue;
          //       const scaledDev = Math.max(-1, Math.min(1, deviation * 2));

          //       const maxRed = 240;
          //       const maxGreen = 220;

          //       // Yellow is baseline for average value
          //       let r = maxRed;
          //       let g = maxGreen;
          //       const b = 60;

          //       if (scaledDev < 0) {
          //         // Below average: Fade out green to shift yellow toward red
          //         g = Math.floor(maxGreen * (1 + scaledDev));
          //       } else {
          //         // Above average: Fade out red to shift yellow toward green
          //         r = Math.floor(maxRed * (1 - scaledDev));
          //       }

          //       return fabricate('div')
          //         .setStyles({
          //           backgroundColor: `rgb(${r}, ${g}, ${b})`,
          //           height: '4px',
          //           flex: '1',
          //         });
          //     }),
          //   );

          return fabricate('Column')
            .setStyles(({ palette }) => ({
              borderBottom: i !== data.length - 1 ? `1px solid ${palette.grey(4)}` : 'none',
            }))
            .setStyles({ width: '100%' })
            .setChildren([groupRow]);
        }),
      );
    });
};
