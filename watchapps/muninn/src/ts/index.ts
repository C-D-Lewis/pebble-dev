import { handleGetSyncInfo, handleSync } from './sync';
import { handlePushTimelinePin } from './timeline';

/**
 * Clear all data phone-side for this watch.
 */
const deleteAll = () => {
  localStorage.clear();
  console.log('Cleared localStorage');
};

Pebble.addEventListener('ready', async (e) => {
  // !!! TEST ONLY
  // deleteAll();
  
  console.log('PebbleKit JS ready!');
  await PebbleTS.sendAppMessage({ READY: 1 });
});

Pebble.addEventListener('appmessage', async (e) => {
  const { payload: dict } = e;
  console.log(`appmessage: ${JSON.stringify(dict)}`);

  try {
    if (dict.PUSH_PIN) await handlePushTimelinePin(dict);
    if (dict.GET_SYNC_INFO) await handleGetSyncInfo();
    if (dict.SYNC_SAMPLE) await handleSync(dict);
    if (dict.SYNC_DELETE) await deleteAll();
  } catch (e) {
    console.log('Failed to handle message');
    console.log(e);
  }
});
