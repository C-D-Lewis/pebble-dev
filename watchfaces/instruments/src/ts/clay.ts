// Clay setup with PKTS
//
// Derived from https://github.com/CometDog/pebble-kite

interface Clay {}
interface ClayConstructor {
  new(config: object[]): Clay;
};

/** Options for complication slots */
const SLOT_OPTIONS = [
  { label: 'None', value: 'NONE' },
  { label: 'Battery', value: 'BATTERY' },
  { label: 'Step count', value: 'STEP_COUNT' },
  { label: 'Next 6hrs rain', value: 'NEXT_RAIN' },
  { label: 'Next 6hrs weather', value: 'NEXT_WEATHER' },
  { label: 'Low/high temperature', value: 'TEMP_RANGE' },
];

const config = [
  { type: 'heading', defaultValue: 'Instruments Settings' },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Complications' },
      {
        type: 'select',
        messageKey: 'CONFIG_SLOT_1',
        defaultValue: 'BATTERY',
        label: 'Slot 1',
        options: SLOT_OPTIONS,
      },
      {
        type: 'select',
        messageKey: 'CONFIG_SLOT_2',
        defaultValue: 'NONE',
        label: 'Slot 2',
        options: SLOT_OPTIONS,
      },
      {
        type: 'select',
        messageKey: 'CONFIG_SLOT_3',
        defaultValue: 'NONE',
        label: 'Slot 3',
        options: SLOT_OPTIONS,
      },
      {
        type: 'select',
        messageKey: 'CONFIG_SLOT_4',
        defaultValue: 'NONE',
        label: 'Slot 4',
        options: SLOT_OPTIONS,
      },
    ],
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Options' },
      {
        type: 'select',
        messageKey: 'CONFIG_THEME',
        defaultValue: 'BLUE',
        label: 'Color Theme',
        options: [
          { label: 'Blue', value: 'BLUE' },
          { label: 'Red', value: 'RED' },
          { label: 'Green', value: 'GREEN' },
          { label: 'Aqua', value: 'AQUA' },
          { label: 'Purple', value: 'PURPLE' },
          { label: 'Orange', value: 'ORANGE' },
          { label: 'Gray', value: 'GRAY' },
        ],
      },
      {
        type: 'select',
        messageKey: 'CONFIG_STEP_GOAL',
        defaultValue: '10000',
        label: 'Step Goal',
        options: [
          { label: '5000', value: '5000' },
          { label: '7500', value: '7500' },
          { label: '10000', value: '10000' },
        ],
      },
      {
        type: 'select',
        messageKey: 'CONFIG_TEMP_UNIT',
        defaultValue: 'C',
        label: 'Temperature Unit',
        options: [
          { label: 'Celsius', value: 'C' },
          { label: 'Farenheit', value: 'F' },
        ],
      },
    ],
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Notes' },
      {
        type: 'text',
        defaultValue: 'Next 6hrs rain/weather shows current hour and 5 upcoming hours, until the last 6 hours of the day where the indicator moves instead.',
      },
      {
        type: 'text',
        defaultValue: 'Ligher blues mean more chance of rain / greater rain intensity.',
      },
    ],
  },
  { type: 'submit', defaultValue: 'Save' },
];

export const setupClay = () => {
  // @ts-ignore
  const Clay: ClayConstructor = require('@rebble/clay');
  new Clay(config);
};
