module.exports = [
  {
    "type": "heading",
    "defaultValue": "Brackets Configuration" ,
    "size": 3
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Options"
      },
      {
        "type": "toggle",
        "messageKey": "BatteryAndBluetooth",
        "label": "Show Battery & BT Status",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "SecondTick",
        "label": "Show Second Tick",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "WeatherStatus",
        "label": "Show Local Weather",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },
      {
        type: 'color',
        label: 'Background',
        messageKey: 'ColorBackground',
        defaultValue: 'FFFFFF'
      },
      {
        type: 'color',
        label: 'Brackets',
        messageKey: 'ColorBrackets',
        defaultValue: '000000'
      },
      {
        type: 'color',
        label: 'Date and Time',
        messageKey: 'ColorDateTime',
        defaultValue: '000000'
      },
      {
        type: 'color',
        label: 'Complications',
        messageKey: 'ColorComplications',
        defaultValue: '000000'
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
];
