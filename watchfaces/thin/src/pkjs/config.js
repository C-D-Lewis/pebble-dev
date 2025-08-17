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
        "type": "text",
        "defaultValue": "Turn additional features on or off."
      },
      {
        "type": "toggle",
        "label": "Show weekday and month",
        "messageKey": "DataKeyDate",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show day of the month",
        "messageKey": "DataKeyDay",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show disconnected indicator",
        "messageKey": "DataKeyBT",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show battery level (hour markers)",
        "messageKey": "DataKeyBattery",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show second hand (uses more power)",
        "messageKey": "DataKeySecondHand",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Show step count",
        "messageKey": "DataKeySteps",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
]
