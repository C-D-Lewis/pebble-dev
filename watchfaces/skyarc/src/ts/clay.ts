// Clay setup with PKTS
//
// Derived from https://github.com/CometDog/pebble-kite

interface Clay {}
interface ClayConstructor {
  new(config: object[]): Clay;
};

const config = [
  { type: 'heading', defaultValue: 'Skyline Configuration' },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Features' },
      {
        type: 'toggle',
        messageKey: 'CONFIG_ANIMATIONS',
        label: 'Show Animations',
        defaultValue: true
      },
      {
        type: 'select',
        messageKey: 'CONFIG_TAP_TIMEOUT',
        defaultValue: '5',
        label: 'Tap/shake Timeout',
        options: [
          { label: '3 seconds', value: '3' },
          { label: '5 seconds', value: '5' },
          { label: '10 seconds', value: '10' }
        ]
      },
      {
        type: 'select',
        messageKey: 'CONFIG_CLOUD_RENDER_MODE',
        defaultValue: '5',
        label: 'Draw cloudy periods as',
        options: [
          { label: 'Striped', value: 'STRIPED' },
          { label: 'Solid', value: 'SOLID' }
        ]
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Colors' },
      { type: 'text', defaultValue: 'Only available on color watches.' },
      {
        type: 'select',
        messageKey: 'CONFIG_COLOR_BG',
        defaultValue: 'GColorOxfordBlue',
        label: 'Background Color',
        options: [
          { label: 'Black', value: 'GColorBlack' },
          { label: 'Oxford Blue', value: 'GColorOxfordBlue' },
          { label: 'Bulgarian Rose', value: 'GColorBulgarianRose' },
          { label: 'Dark Green', value: 'GColorDarkGreen' },
          { label: 'Chrome Yellow', value: 'GColorChromeYellow' }
        ]
      },
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Units' },
      {
        type: 'select',
        messageKey: 'CONFIG_TEMP_UNIT',
        defaultValue: 'C',
        label: 'Temperature Unit',
        options: [
          { label: 'Celsius', value: 'C' },
          { label: 'Farenheit', value: 'F' }
        ]
      },
      {
        type: 'select',
        messageKey: 'CONFIG_WIND_UNIT',
        defaultValue: 'MPH',
        label: 'Wind Speed Unit',
        options: [
          { label: 'MPH', value: 'MPH' },
          { label: 'KPH', value: 'KPH' }
        ]
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
