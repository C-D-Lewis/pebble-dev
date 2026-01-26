/** Maximum stored items */
var MAX_ITEMS = 64;
/** LocalStorage key - data history */
var LS_KEY_HISTORY = 'history';
/** Map of AppMessage fields to JS sample model */
var FIELD_MAP = {
  SAMPLE_TIMESTAMP: 'timestamp',
  SAMPLE_RESULT: 'result',
  SAMPLE_CHARGE_PERC: 'chargePerc',
  SAMPLE_TIME_DIFF: 'timeDiff',
  SAMPLE_CHARGE_DIFF: 'chargeDiff',
  SAMPLE_RATE: 'rate'
};
/** All expected keys */
var MESSAGE_KEYS = Object.keys(FIELD_MAP);

/**
 * Save the history of samples.
 */
function saveHistory(history) {
  localStorage.setItem(LS_KEY_HISTORY, JSON.stringify(history));
}

/**
 * Load the history of samples.
 */
function loadHistory() {
  try {
    return JSON.parse(localStorage.getItem(LS_KEY_HISTORY) || '[]');
  } catch (e) {
    console.error('Failed to load history');
    console.error(e);
    throw e;
  }
}

/**
 * Send the watch the last seen timestamp so updates can be incremental.
 */
function handleGetSyncInfo() {
  var history = loadHistory();
  // console.log(JSON.stringify(localStorage.getItem('history')));

  var timestamp = -1; // STATUS_EMPTY
  if (history.length > 0) {
    timestamp = history[0].timestamp;
  }

  Pebble.sendAppMessage({
    SYNC_TIMESTAMP: timestamp,
    SYNC_COUNT: history.length
  });
}

/**
 * Importing data should be as robust as possible.
 *
 * TODO: How do we handle adding new fields?
 */
function handleSync(dict) {
  var history = loadHistory();
  var timestamp = dict.SAMPLE_TIMESTAMP;

  // Manual loop replaces .find() which coreapp doesn't like
  for (var i = 0; i < history.length; i++) {
    if (history[i].timestamp === timestamp) {
      console.log('Skipping duplicate: ' + timestamp);
      return;
    }
  }

  // Construct local sample
  var sample = {};
  for (var j = 0; j < MESSAGE_KEYS.length; j++) {
    var key = MESSAGE_KEYS[j];
    if (typeof dict[key] === 'undefined') throw new Error('Missing field: ' + key);
    if (!FIELD_MAP[key]) throw new Error('Unknown key: ' + key);

    sample[FIELD_MAP[key]] = dict[key];
  }
  history.push(sample);

  // Sort to match watch order and limit length
  history = history
    .sort(function(a, b) { return a.timestamp > b.timestamp ? -1 : 1 })
    .slice(0, MAX_ITEMS);

  saveHistory(history);
  console.log('Saved new sample: ' + JSON.stringify(sample));
}

module.exports = {
  handleGetSyncInfo,
  handleSync,
};
