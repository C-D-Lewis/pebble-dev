var timeline = require('pebble-timeline-js');

/** Seconds in a day */
var SECONDS_PER_DAY = 60 * 60 * 24;

/** Predicted low charge time pin ID */
var PIN_ID_PREDICTION = 'muninn-prediction';

function handlePushTimelinePin(dict) {
  var days = dict.DAYS_REMAINING;
  var rate = dict.DISCHARGE_RATE;

  // Put the pin in the future at noon
  var date = new Date(Date.now() + (days * SECONDS_PER_DAY * 1000));
  date.setHours(12);
  date.setMinutes(0);
  date.setSeconds(0);

  // Create the pin
  var pin = {
    id: PIN_ID_PREDICTION,
    time: date.toISOString(),
    layout: {
      type: 'genericPin',
      title: 'Time to charge!',
      body: 'Muninn predicts you will need to charge soon (Est. ' + rate + '%/day)',
      // TODO: When SDK is fixed, use publishedMedia for custom icon
      tinyIcon: 'system://images/GENERIC_WARNING'
    }
  };

  console.log('Inserting pin: ' + JSON.stringify(pin));
  timeline.insertUserPin(pin, function(responseText) { 
    console.log('Result: ' + responseText);
  });
}

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function(e) {
  var dict = e.payload;
  console.log('dict: ' + JSON.stringify(dict));

  if (dict.DAYS_REMAINING) {
    handlePushTimelinePin(dict);
    return;
  }

  if (dict.EXPORT) {
    // TODO: How to serialize the C structs for AppMessage transport in an easily
    //       extendable way?
  }
});
