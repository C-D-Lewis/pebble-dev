module.exports = [
  { 
    "type": "heading", 
    "defaultValue": "Thin Configuration" ,
    "size": 3
  }, 
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Features"
      },
      {
        "type": "toggle",
        "label": "Show current date",
        "messageKey": "EnableDate",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show disconnected indicator",
        "messageKey": "EnableBT",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show battery level (hour markers)",
        "messageKey": "EnableBattery",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show second hand (uses more power)",
        "messageKey": "EnableSecondHand",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show step count",
        "messageKey": "EnableSteps",
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
        defaultValue: '000000'
      },
      {
        type: 'color',
        label: 'Hour/minutes hand',
        messageKey: 'ColorHourMinutes',
        defaultValue: 'AAAAAA'
      },
      {
        type: 'color',
        label: 'Seconds hand',
        messageKey: 'ColorSeconds',
        defaultValue: 'AA0000'
      },
      {
        type: 'color',
        label: 'Notches',
        messageKey: 'ColorNotches',
        defaultValue: 'FFFFFF'
      },
      {
        type: 'color',
        label: 'Day of the month',
        messageKey: 'ColorMonthDay',
        defaultValue: 'FFAA00'
      },
      {
        type: 'color',
        label: 'Date',
        messageKey: 'ColorDate',
        defaultValue: 'FFFFFF'
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
]
