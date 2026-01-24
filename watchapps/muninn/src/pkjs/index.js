const { handlePushTimelinePin } = require('./timeline');
const { handleExport, handleGetLastTimestamp } = require('./export');

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');

  Pebble.sendAppMessage({ READY: 1 });
});

Pebble.addEventListener('appmessage', function(e) {
  var dict = e.payload;
  // console.log('appmessage: ' + JSON.stringify(dict));

  try {
    // Update timeline pin
    if (dict.PUSH_PIN) {
      handlePushTimelinePin(dict);
      return;
    }

    // Tell watch the last sample we saw
    if (dict.GET_LAST_TIMESTAMP) {
      handleGetLastTimestamp();
      return;
    }

    // Handle a sample sent from the watch
    if (dict.EXPORT_TIMESTAMP) {
      handleExport(dict);
      return;
    }
  } catch (e) {
    console.log('Failed to handle message');
    console.log(e);
  }
});
