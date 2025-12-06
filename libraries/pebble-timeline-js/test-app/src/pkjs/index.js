var timelinejs = require('pebble-timeline-js');

var future = new Date();
future.setHours(future.getHours() + 1);

var pin = {
  id: 'example-pin-generic',
  time: future,
  layout: {
    type: 'genericPin',
    title: 'timelinejs pin test',
    body: 'Created at ' + new Date().toISOString(),
    tinyIcon: 'system://images/NOTIFICATION_FLAG'
  }
};

Pebble.addEventListener('ready', function() {
  console.log('Ready! Inserting user pin ' + JSON.stringify(pin));

  timelinejs.insertUserPin(pin, function(responseText) {
    console.log('Result: ' + responseText);
  });
});
