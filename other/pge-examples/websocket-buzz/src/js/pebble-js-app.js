/******************************** Config **************************************/

var DEBUG = true;
var VERBOSE = true;
var NUM_APPINFO_KEYS = 16;  // Number of keys declared in appinfo.json 
                            // with values of 0 -> NUM_APPINFO_KEYS

/********************* PGE WS Module JS - DO NOT MODIFY ***********************/

var webSocket = null;

function logVerbose(message) {
  if(VERBOSE) console.log(message);
}

function logDebug(message) {
  if(DEBUG) console.log(message); 
}

function log(message) {
  console.log(message);
}

function hasKey(dict, key) {
  return typeof dict.payload[key] !== 'undefined';
}

var getValue = function(dict, key) {
  return hasKey(dict, key) ? dict.payload[key] : null;
}

function sendToPebble(dict) {
  Pebble.sendAppMessage(dict, function() {
    logVerbose('Success: ' + JSON.stringify(dict));
  }, function() {
    logVerbose('Failed: ' + JSON.stringify(dict));
  });
}

function connectToServer(url) {
  // Url. Example: ws://localhost:5000
  webSocket = new WebSocket(url);
  webSocket.onopen = function(event) { 
    logVerbose('Connection opened!');
    if(event.data) {
      onSocketOpen(event.data); 
      PGEWSSocketProtocol(event.data);
    }
  };
  webSocket.onclose = function(event) { 
    logVerbose('onclose');
    sendToPebble({ 'PGE_WS_URL': 0 });
    onSocketClose(); 
  };
  webSocket.onmessage = function(event) { 
    logVerbose('onmessage');
    if(event.data) {
      if(!PGEWSSocketProtocol(event.data)) {
        // This was not a protocol message
        onSocketMessage(event.data);
      }
    } else {
      logVerbose('onmessage with no data!');
    }
  };
  webSocket.onerror = function(event) { 
    logVerbose('onerror');
    sendToPebble({ 'PGE_WS_URL': 0 });
    onSocketError(); 
  };
}

function PGEWSAppMessageProtocol(dict) {
  logVerbose('handling keys: ' + JSON.stringify(dict.payload));

  // WS URL from C and connect
  if(hasKey(dict, 'PGE_WS_URL')) {
    var url = getValue(dict, 'PGE_WS_URL');
    logDebug('Got WS URL: ' + url);
    connectToServer(url);
  }
}

function PGEWSForwardToServer(dict) {
  // Developer dictionary data to send to server
  if(webSocket) {
    var outgoing = {};

    // Check all appinfo.json keys
    for(var i = 0; i < NUM_APPINFO_KEYS; i += 1) {
      var key = 'PGE_WS_KEY_' + i; // String keys
      if(hasKey(dict, key)) {
        outgoing[key] = getValue(dict, key);
        logVerbose('Added key: ' + key + ', value: ' + outgoing[key]);
      }
    }

    var packet = JSON.stringify(outgoing);
    if(packet.length > 4) {
      // Send this data
      logVerbose('Forwarding data to server: ' + packet);
      webSocket.send(packet);
    }
  } else {
    logVerbose('Data send requested, but webSocket is not ready');
  }
}

function PGEWSForwardToPebble(data) {
  var json = JSON.parse(data);
  var outgoing = {};

  for(var i = 0; i < NUM_APPINFO_KEYS; i += 1) {
    var key = 'PGE_WS_KEY_' + i;
    if(json[key]) {
      outgoing[key] = parseInt(json[key]);
    }
  }
  logVerbose('Forwarding data to Pebble: ' + JSON.stringify(outgoing));
  sendToPebble(outgoing);
}

var PGEWSSocketProtocol = function(data) {
  var json = JSON.parse(data);

  if(json.id) {
    sendToPebble({
      'PGE_WS_URL': 1, // success
      'PGE_WS_CLIENT_ID': parseInt(json.id)
    });
    return true;
  }

  return false;
}

function PGEWSReady() {
  sendToPebble({ 'PGE_WS_READY': 1 });
}

/**************************** End PGE WS Module JS ****************************/


/******************* Developer Implemented PGE WS callbacks *******************/

function onSocketOpen(data) {

}

function onSocketClose() {

}

function onSocketMessage(data) {
  // Auto-forward to Pebble
  PGEWSForwardToPebble(data);
}

function onSocketError() {
  
}

/****************************** PebbleKit JS **********************************/

Pebble.addEventListener('ready', function() {
  console.log('PebbleKitJS ready');
  PGEWSReady();
});

Pebble.addEventListener('appmessage', function(dict) {
  PGEWSAppMessageProtocol(dict);

  // Auto-forward to server
  PGEWSForwardToServer(dict);
});