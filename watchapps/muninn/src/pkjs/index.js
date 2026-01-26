const { handlePushTimelinePin } = require('./timeline');
const { handleSync, handleGetSyncInfo } = require('./sync');

/**
 * Clear all data phone-side.
 */
function deleteAll() {
  localStorage.clear();
  console.log('Cleared localStorage');
}

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
  Pebble.sendAppMessage({ READY: 1 });
});

Pebble.addEventListener('appmessage', function(e) {
  var dict = e.payload;
  // console.log('appmessage: ' + JSON.stringify(dict));

  try {
    if (dict.PUSH_PIN) handlePushTimelinePin(dict);
    if (dict.GET_SYNC_INFO) handleGetSyncInfo();
    if (dict.SYNC_SAMPLE) handleSync(dict);
    if (dict.SYNC_DELETE) deleteAll();
  } catch (e) {
    console.log('Failed to handle message');
    console.log(e);
  }
});
