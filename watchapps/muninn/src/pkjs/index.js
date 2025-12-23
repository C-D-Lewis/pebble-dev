var timeline = require('pebble-timeline-js');

/** Seconds in a day */
var SECONDS_PER_DAY = 60 * 60 * 24;

/** Predicted low charge time pin ID */
var PIN_ID_PREDICTION = 'muninn-prediction';

function handlePushTimelinePin(dict) {
  var days = dict.DAYS_REMAINING;
  var rate = dict.DISCHARGE_RATE;

  // Put the pin in the future at noon, or today late if no days remaining
  var target = new Date(Date.now() + (days * SECONDS_PER_DAY * 1000));
  var targetHour = (days == 0 && new Date().getHours() > 12) ? 23 : 12;
  target.setHours(targetHour);
  target.setMinutes(0);
  target.setSeconds(0);

  // Create the pin
  var pin = {
    id: PIN_ID_PREDICTION,
    time: target.toISOString(),
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
    // Coreapp returns nothing here, we assume this callback means OK
    console.log('Result: ' + responseText);

    Pebble.sendAppMessage({ PIN_SET: 1 }, function() {
      console.log('Notified app of pin set');
    }, function() {
      console.log('Error notifying app of pin set');
    });
  });
}

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function(e) {
  var dict = e.payload;
  console.log('dict: ' + JSON.stringify(dict));

  if (typeof dict.DAYS_REMAINING !== 'undefined') {
    handlePushTimelinePin(dict);
    return;
  }

  if (dict.EXPORT) {
    // TODO: How to serialize the C structs for AppMessage transport in an easily
    //       extendable way?
  }
});
