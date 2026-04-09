// Clay setup with PKTS - see README.md
//
//   Derived from: https://github.com/CometDog/pebble-kite/blob/main/src/ts-config/config.ts
//   This file is included by rollup after PKTS has transformed the main TS code

export default function buildClayConfig() {
  return [
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
};
