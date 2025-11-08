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
        "type": "text",
        "defaultValue": "Some values require closing and re-opening the watchface."
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
    "type": "submit",
    "defaultValue": "Save"
  }
];
