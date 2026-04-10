// Clay setup with PKTS
//
// Derived from https://github.com/CometDog/pebble-kite

interface Clay {}
interface ClayConstructor {
  new(config: object[]): Clay;
};

const config = [
  {
    type: 'heading',
    defaultValue: 'Skyline Configuration'
  },
  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Units'
      },
      {
        type: 'select',
        messageKey: 'CONFIG_TEMP_UNIT',
        defaultValue: 'celsius',
        label: 'Temperature Unit',
        options: [
          { label: 'Celsius', value: 'C' },
          { label: 'Farenheit', value: 'F' }
        ]
      }
    ]
  },
  {
    type: 'submit',
    defaultValue: 'Save'
  }
];

export const setupClay = () => {
  // @ts-ignore
  const Clay: ClayConstructor = require('@rebble/clay');
  new Clay(config);
};
