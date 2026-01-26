var timeline = require('pebble-timeline-js');

/** Seconds in a day */
var SECONDS_PER_DAY = 60 * 60 * 24;
/** Predicted low charge time pin ID */
var PIN_ID_PREDICTION = 'muninn-prediction';

/**
 * Handle request to push a timeline pin.
 */
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
  timeline.insertUserPin(pin, function(responseText) { });
}

module.exports = {
  handlePushTimelinePin,
};
