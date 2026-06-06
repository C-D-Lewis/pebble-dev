import { Fabricate, FabricateComponent } from 'fabricate.js';
import { AppState, GlobalStatItem } from '../types.ts';
import { Annotation, ImageButton } from './index.ts';
import Theme from '../theme.ts';

declare const fabricate: Fabricate<AppState>;

/**
 * StatView component.
 *
 * @param {object} props - Component props.
 * @param {string} props.label - Stat label.
 * @param {number} props.value - Stat value.
 * @returns {FabricateComponent} Fabricate component.
 */
const StatView = ({ label, value }: { label: string, value: string }) => fabricate('Column')
  .setChildren([
    fabricate('Text')
      .setStyles(({ palette }) => ({
        color: palette.text,
        fontSize: '1.2rem',
        textAlign: 'center',
        marginBottom: '0px',
      }))
      .setText(value),
    fabricate('Text')
      .setStyles(({ palette }) => ({
        color: palette.grey(9),
        fontSize: '0.8rem',
        textAlign: 'center',
        marginTop: '0',
      }))
      .setText(label),
  ]);

/**
 * GlobalStatsList component.
 *
 * @param {object} props - Component props.
 * @param {string} props.field - Global stats list to use.
 * @returns {FabricateComponent} Fabricate component.
 */
const GlobalStatsList = (
  { field }: { field: 'models' | 'platforms' },
) => fabricate('Column')
  .setStyles({ marginBottom: '12px' })
  .onCreate((el, state) => {
    const data = state.globalStats[field];

    el.addChildren(
      data.map((group: GlobalStatItem) => {
        const {
          count,
          groupName,
          avgBatteryLife,
          avgRate,
        } = group;
        const stateKey: `expanded-${string}` = fabricate.buildKey('expanded', groupName) as `expanded-${string}`;

        const nameRow = fabricate('Row')
          .setStyles({ cursor: 'pointer' })
          .setChildren([
            fabricate('Text')
              .setStyles(({ palette }) => ({
                color: palette.text,
                fontWeight: 'bold',
                fontSize: '0.9rem',
                marginLeft: '10px',
                marginBottom: '6px',
              }))
              .setText(groupName),
            ImageButton({ src: 'assets/images/expand.png' })
              .setStyles({ marginLeft: 'auto', marginRight: '10px' })
              .onCreate(() => fabricate.update(stateKey, false))
              .onUpdate((el2, state2) => {
                const expanded = state2[stateKey];
                el2.setStyles({ transform: expanded ? 'rotate(180deg)' : 'rotate(0deg)' });
              }, [stateKey]),
          ])
          .onClick((_: FabricateComponent<AppState>, state2: AppState) => {
            fabricate.update(stateKey, !state2[stateKey]);
          });

        const statsRow = fabricate('Row')
          .setStyles({
            justifyContent: 'space-around',
            borderTop: `1px solid ${Theme.palette.grey(4)}`,
            transition: '0.3s',
            height: '0px',
            overflow: 'hidden',
          })
          .setChildren([
            StatView({ label: 'Users', value: String(count) }),
            StatView({ label: 'Avg. Life', value: `${avgBatteryLife} days` }),
            StatView({ label: 'Avg. Rate', value: `${avgRate}%` }),
          ])
          .onUpdate((el2, state2) => {
            el2.setStyles({ height: state2[stateKey] ? '58px' : '0px' });
          }, [stateKey]);

        return fabricate('Column')
          .setStyles(({ palette }) => ({
            border: `1px solid ${palette.grey(5)}`,
            borderRadius: '4px',
            marginBottom: '4px',
          }))
          .setStyles({ width: '100%' })
          .setChildren([nameRow, statsRow]);
      }),
    );
  });

/**
 * GlobalStatsView component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const GlobalStatsView = () => fabricate('Column')
  .setStyles({ marginTop: '10px' })
  .onUpdate(async (el, state) => {
    const { globalStats } = state;
    const { historyCount, updatedAt  } = globalStats;

    const date = new Date(updatedAt);

    el.setChildren([
      fabricate('Column')
        .setChildren([
          GlobalStatsList({ field: 'models' }),
          Annotation().setText(`From ${historyCount} users (updated: ${date.toLocaleString()})`),
        ]),
    ]);
  }, [fabricate.StateKeys.Created, 'globalStats']);
