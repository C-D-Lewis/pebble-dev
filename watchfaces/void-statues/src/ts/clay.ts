// Clay setup with PKTS
//
// Derived from https://github.com/CometDog/pebble-kite

interface Clay {}
interface ClayConstructor {
  new(config: object[]): Clay;
};

const config = [
  { type: 'heading', defaultValue: 'Void Statues Configuration' },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Colors' },
      {
        type: 'color',
        messageKey: 'DAY_BG_COLOR',
        defaultValue: '#FFFFFF',
        label: 'Day Background Color',
        allowGray: true
      },
      {
        type: 'color',
        messageKey: 'DAY_BLOCK_COLOR',
        defaultValue: '#000000',
        label: 'Day Block Color',
        allowGray: true
      },
      {
        type: 'color',
        messageKey: 'DAY_SHADOW_COLOR',
        defaultValue: '#AAAAAA',
        label: 'Day Shadow Color',
        allowGray: true
      },
      {
        type: 'color',
        messageKey: 'DAY_VOID_COLOR',
        defaultValue: '#555555',
        label: 'Day Void Color',
        allowGray: true
      },
      {
        type: 'color',
        messageKey: 'NIGHT_BG_COLOR',
        defaultValue: '#000000',
        label: 'Night Background Color',
        allowGray: true
      },
      {
        type: 'color',
        messageKey: 'NIGHT_BLOCK_COLOR',
        defaultValue: '#FFFFFF',
        label: 'Night Block Color',
        allowGray: true
      },
      {
        type: 'color',
        messageKey: 'NIGHT_SHADOW_COLOR',
        defaultValue: '#555555',
        label: 'Night Shadow Color',
        allowGray: true
      },
      {
        type: 'color',
        messageKey: 'NIGHT_VOID_COLOR',
        defaultValue: '#AAAAAA',
        label: 'Night Void Color',
        allowGray: true
      }
    ]
  },
  { type: 'submit', defaultValue: 'Save' }
];

export const setupClay = () => {
  // @ts-ignore
  const Clay: ClayConstructor = require('@rebble/clay');
  new Clay(config);
};
