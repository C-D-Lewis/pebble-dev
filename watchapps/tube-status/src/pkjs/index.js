var VERSION = '3.4';
var VERBOSE = true;
var TOPIC_PINS = 'delays';
var TOPIC_NOTIFS = 'notifs';

var AppMessageValueTimeout = 2;

var SubscriptionStateUnsubscribed = 0,
    SubscriptionStateSubscribed = 1;

/********************************** Helpers ***********************************/

function Log(content) {
  if(VERBOSE) console.log(content);
};

var hasKey = function(dict, key) {
  return typeof dict.payload[key] !== 'undefined';
};

// Request-like shim
function request(url, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    callback(this.responseText);
  };
  xhr.open('GET', url);
  xhr.send();
};

/******************************* New Unified API ******************************/

function downloadNewAPI() {
  request('https://api.tfl.gov.uk/line/mode/tube/status', function(responseText) {
    console.log('Download from unified API complete!');

    // Get names
    var lineStates = [];
    var lines = JSON.parse(responseText);
    for(var i = 0; i < lines.length; i++) {
      // Yay JSON!
      lineStates[i] = lines[i].lineStatuses[0].statusSeverityDescription;
    }

    sendToPebble(lineStates);
  });
  Log('Downloading from unified API...');
}

/************************************* App ************************************/

function sendToPebble(lineStates) {
  // TODO Don't use order of values
  var dict = {
    'LineTypeBakerloo': lineStates[0], 
    'LineTypeCentral': lineStates[1],
    'LineTypeCircle': lineStates[2],
    'LineTypeDistrict': lineStates[3],
    'LineTypeHammersmithAndCity': lineStates[4],
    'LineTypeJubilee': lineStates[5],
    'LineTypeMetropolitan': lineStates[6],
    'LineTypeNorthern': lineStates[7],
    'LineTypePicadilly': lineStates[8],
    'LineTypeVictoria': lineStates[9],
    'LineTypeWaterlooAndCity': lineStates[10]
  };
  console.log(JSON.stringify(dict));

  Pebble.sendAppMessage(dict, function(e) {
    console.log('Sent to Pebble!');
  }, function(e) {
    console.log('Send failed!');
  });
}

/******************************** PebbleKit JS ********************************/

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready! Version ' + VERSION);

  // Inform that JS is ready
  Pebble.sendAppMessage({'AppMessageKeyJSReady': 1}, function(e) {
    Log('Send ready event successfully');
  }, function(e) {
    console.log('Failed to send ready event!');
  });
});

function setTopicState(topic, subscribe) {
  if(subscribe) {
    Pebble.timelineSubscribe(topic,
      function(success) { Log('setTopicState(): Subscribe to ' + topic + ' OK'); },
      function(error)   { Log('setTopicState(): Subscribe error: ' + error); }
    );
  } else {
    Pebble.timelineUnsubscribe(topic,
      function(success) { Log('setTopicState(): Unsubscribe from ' + topic + ' OK'); },
      function(error)   { Log('setTopicState(): Unsubscribe error: ' + error); }
    );
  }
}

function updateSubscriptions(state) {
  switch(state) {
    case SubscriptionStateUnsubscribed:
      setTopicState(TOPIC_PINS, false);
      setTopicState(TOPIC_NOTIFS, false);
      break;
    case SubscriptionStateSubscribed:
      setTopicState(TOPIC_PINS, true);
      setTopicState(TOPIC_NOTIFS, false); // Never notifs
      break;
    default: break;
  }

  // Get new data now that settings window has closed
  downloadNewAPI();
}

Pebble.addEventListener('appmessage', function(dict) {
  Log('Got appmessage: ' + JSON.stringify(dict.payload));

  // Watch wants tube status
  if(hasKey(dict, 'AppMessageKeyJSReady')) {
    Log('Data request receieved.');
    downloadNewAPI();
  } 

  // Watch weants to update subscription state
  if(hasKey(dict, 'AppMessageKeySubscriptionState')) {
    Log('Updating subscription...');
    updateSubscriptions(dict.payload['AppMessageKeySubscriptionState']);
  }
});
