const { handlePushTimelinePin } = require('./timeline');
const { handleSyncUpdate } = require('./sync');

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function(e) {
  var dict = e.payload;
  console.log('appmessage: ' + JSON.stringify(dict));

  if (dict.PUSH_PIN) {
    handlePushTimelinePin(dict);
    return;
  }

  if (dict.SYNC) {
    handleSyncUpdate(dict);
  }
});
