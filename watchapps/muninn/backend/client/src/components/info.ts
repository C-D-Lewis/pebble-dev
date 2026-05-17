import { Fabricate, FabricateComponent } from 'fabricate.js';
import { AppState } from '../types.ts';

declare const fabricate: Fabricate<AppState>;

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
        fontSize: '0.8rem',
        margin: '2px 4px',
      }))
      .setText(label),
    fabricate('Text')
      .setStyles({ color: '#ccc', fontSize: '0.8rem', fontFamily: 'monospace' })
      .setText(value),
  ]);

/**
 * InfoChips component.
 *
 * @returns {FabricateComponent} Fabricate component.
 */
export const InfoChips = () => fabricate('Column')
  .setStyles({ padding: '4px' })
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
