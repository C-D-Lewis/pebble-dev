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
        "defaultValue": "Colors"
      },
      {
        "type": "text",
        "defaultValue": "Only available on compatible platforms."
      },
      {
        "type": "color",
        "label": "Background",
        "messageKey": "BackgroundColor",
        "defaultValue": "FFFFFF"
      },
      {
        "label": "Date",
        "type": "color",
        "messageKey": "DateColor",
        "defaultValue": "000000"
      },
      {
        "label": "Time",
        "type": "color",
        "messageKey": "TimeColor",
        "defaultValue": "000000"
      },
      {
        "label": "Bracket",
        "type": "color",
        "messageKey": "BracketColor",
        "defaultValue": "000000"
      },
      {
        "label": "Line",
        "type": "color",
        "messageKey": "LineColor",
        "defaultValue": "7D7D7D"
      },
      {
        "label": "Battery Complication",
        "type": "color",
        "messageKey": "ComplicationColor",
        "defaultValue": "000000"
      }
    ]
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
        "messageKey": "BatteryMeter",
        "label": "Show Battery Meter",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "BluetoothAlert",
        "label": "Bluetooth Alert",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "DashedLine",
        "label": "Show Dashed Line",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "SecondTick",
        "label": "Show Second Tick",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
];
