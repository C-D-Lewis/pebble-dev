import { Fabricate, FabricateComponent } from 'fabricate.js';
import { AppState } from '../types.ts';
import { STATUS_EMPTY } from '../constants.ts';
import { Annotation } from './index.ts';

declare const fabricate: Fabricate<AppState>;

/**
 * StatView component.
 *
 * @param {object} props - Component props.
 * @param {string} props.label - Stat label.
 * @param {string} props.value - Stat value.
 * @param {string} [props.note] - Note if needed.
 * @returns {FabricateComponent} Fabricate component.
 */
const StatView = (
  { label, value, note }: { label: string, value: string, note?: string },
) => {
  const col = fabricate('Column')
    .setStyles(({ palette }) => ({
      backgroundColor: palette.grey(4),
      borderRadius: '5px',
      padding: '2px 4px 8px 4px',
      margin: '8px',
      flex: '1',
      height: 'fit-content',
    }))
    .setChildren([
      fabricate('Text')
        .setStyles({
          color: '#ccc',
          fontSize: '0.9rem',
          textAlign: 'center',
        })
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

  if (note) col.addChildren([Annotation().setText(note)]);

  return col;
};

/**
 * StatsList component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const StatsList = () => fabricate('Column')
  .setStyles({ marginTop: '4px 8px' })
  .onUpdate((el, state) => {
    const { stats, model, globalStats } = state;
    const {
      count,
      totalDays,
      allTimeRate,
      lastWeekRate,
      numCharges,
      mtbc,
    } = stats;

    // ROund these to one decimal place
    const batteryDays = Math.floor(100 / allTimeRate);
    const lastWeekBatteryDays = Math.floor(100 / lastWeekRate);

    // Compare battery life against all users with this model
    const modelStats = globalStats.models.find((p) => p.names.includes(model));
    let compareDaysStr = '-';
    let compareRateStr = '-';
    if (modelStats) {
      // TODO: Clean this up
      // const diffDays = batteryDays - modelStats.avgBatteryLife;
      const diffDays = batteryDays - modelStats.medianBatteryLife;
      let operator = diffDays >= 0 ? 'above' : 'below';
      compareDaysStr = `${Math.abs(diffDays)} ${operator} average for this model`;

      const diffRate = allTimeRate - modelStats.avgRate;
      operator = diffRate >= 0 ? 'above' : 'below';
      compareRateStr = `${Math.abs(diffRate)}% ${operator} average for this model`;
    }

    // Could be empty if not much data
    const lvarValue = lastWeekRate !== STATUS_EMPTY ? `${lastWeekRate}% per day` : '-';
    const mtbcValue = mtbc !== STATUS_EMPTY ? `${mtbc} days` : '-';
    const lwelValue = lastWeekRate !== STATUS_EMPTY
      ? `${lastWeekBatteryDays} days`
      : '-';

    el.setChildren([
      fabricate('Row')
        .setChildren([
          StatView({ label: 'Samples', value: String(count) }),
          StatView({ label: 'Duration', value: `${totalDays} days` }),
        ]),
      fabricate('Row')
        .setChildren([
          StatView({
            label: 'Est. Battery Life',
            value: `${batteryDays} days`,
            note: compareDaysStr,
          }),
          StatView({
            label: 'Avg. Discharge Rate',
            value: `${allTimeRate}% per day`,
            note: compareRateStr,
          }),
        ]),
      fabricate('Row')
        .setChildren([
          StatView({ label: 'Last Week Est. Life', value: lwelValue }),
          StatView({ label: 'Last Week Avg. Rate', value: lvarValue }),
        ]),
      fabricate('Row')
        .setChildren([
          StatView({ label: 'Charge Events', value: `${numCharges} events` }),
          StatView({ label: 'Avg. Charge Interval', value: mtbcValue }),
        ]),
    ]);
  }, [fabricate.StateKeys.Created, 'globalStats']);
