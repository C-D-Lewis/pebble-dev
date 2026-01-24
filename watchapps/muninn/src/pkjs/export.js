/** Maximum stored items */
var MAX_ITEMS = 64;
/** LocalStorage key - data history */
var LS_KEY_HISTORY = 'history';
/** Fields required in each AppMessage sample payload */
var EXPORT_KEYS = [
  'EXPORT_TIMESTAMP',
  'EXPORT_RESULT',
  'EXPORT_CHARGE_PERC',
  'EXPORT_TIME_DIFF',
  'EXPORT_CHARGE_DIFF',
  'EXPORT_RATE'
];
/** Map of AppMessage fields to JS sample model */
var FIELD_MAP = {
  EXPORT_TIMESTAMP: 'timestamp',
  EXPORT_RESULT: 'result',
  EXPORT_CHARGE_PERC: 'chargePerc',
  EXPORT_TIME_DIFF: 'timeDiff',
  EXPORT_CHARGE_DIFF: 'chargeDiff',
  EXPORT_RATE: 'rate'
};

function saveHistory(history) {
  localStorage.setItem(LS_KEY_HISTORY, JSON.stringify(history));
  // console.log('Saved history (' + history.length + ' items)');
}

function loadHistory() {
  try {
    var history = JSON.parse(localStorage.getItem(LS_KEY_HISTORY) || '[]');
    // console.log('Loaded history (' + history.length + ' items)');
    return history;
  } catch (e) {
    console.error('Failed to load history');
    console.error(e);
    throw e;
  }
}

/**
 * Send the watch the last seen timestamp so updates can be incremental.
 */
function handleGetLastTimestamp() {
  var lastTimestamp = -1;

  var history = loadHistory();
  // console.log(JSON.stringify(localStorage.getItem('history')));
  if (history.length > 0) {
    lastTimestamp = history[history.length - 1].timestamp;
  }

  Pebble.sendAppMessage({ LAST_TIMESTAMP: lastTimestamp });
}

/**
 * Importing data should be as robust as possible.
 *
 * TODO: How do we handle adding new fields?
 */
function handleExport(dict) {
  var history = loadHistory();

  // Have we seen this before?
  var timestamp = dict.EXPORT_TIMESTAMP;
  if (history.find(function (p) { return p.timestamp === timestamp })) {
    console.log('Skipping duplicate: ' + timestamp);
    return;
  }

  // Construct local sample
  var sample = {};
  for (var i = 0; i < EXPORT_KEYS.length; i++) {
    let key = EXPORT_KEYS[i];
    if (typeof dict[key] === 'undefined') throw new Error('Missing field: ' + key);
    if (!FIELD_MAP[key]) throw new Error('Unknown key: ' + key);

    sample[FIELD_MAP[key]] = dict[key];
  }
  history.push(sample);
  console.log('Saved new sample: ' + JSON.stringify(sample));

  // TODO: Which order on timestamp should we sort?

  saveHistory(history);
}

module.exports = {
  handleGetLastTimestamp,
  handleExport,
};
