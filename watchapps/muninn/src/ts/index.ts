import { testStats } from './test/tests';
import { handleGetSyncInfo, handleSync } from './sync';
import { handlePushTimelinePin } from './timeline';
import { ensureUploadId, uploadHistory } from './upload';

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
  testStats();

  // Prevent display issues by getting this as early as possible
  await ensureUploadId();
  
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
    if (dict.UPLOAD_HISTORY) await uploadHistory();
  } catch (e) {
    console.log('Failed to handle message');
    console.log(e);
  }
});

Pebble.addEventListener('showConfiguration', async () => {
  const id = await ensureUploadId();

  // Try and upload freshest data for the user
  await uploadHistory();

  // isAppConfigPage hides some elements that can't be clicked in the config page webview
  Pebble.openURL(`https://muninn.chrislewis.me.uk?id=${id}&isAppConfigPage=true`);
});
