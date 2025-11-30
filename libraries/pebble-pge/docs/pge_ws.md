# PGE WebSocket Documentation

This module is aimed at enabling easy communication from a game to an
implementation of the `pge-ws-server` example Node.js server.


## About AppMessage Keys

Due to limitations in PebbleKit JS, AppMessage keys must be declared in
`appinfo.json`. To accomodate for this, 16 intial keys have been declared, named
`PGE_WS_KEY_X`, where `X` is `0` to `15`. You should repurpose these for app-
specific data, as shown in the example of `onClientConnected()` shown below. If
you need more, add them to your own implementation. If you do, don't forget to
update the value of `PGE_WS_NUM_KEYS` in `pge/modules/pge_ws.h`!

You should repurpose your keys in the following manner:

```
#define TOTAL_PLAYERS PGE_WS_KEY_0
```


## Preparing the Server

Edit `pge-ws-server/app.js` to implement the `onClientConnected()` and
`onClientMessage()` functions to turn client communication into Pebble
AppMessages. For example:

```
function onClientConnected(socket) {
  // Send text to server
  socket.send(JSON.stringify({ 'PGE_WS_KEY_0': 42 }));
}

function onClientDisconnected(client) {

}

function onClientMessage(socket, data) {
  var json = JSON.parse(data);

  // Process data from server
  if(json['PGE_WS_KEY_0']) {
    var someValue = json['PGE_WS_KEY_0'];

    // Send data on to Pebble
    Pebble.sendAppMessage({ 'PGE_WS_KEY_0': parseInt(someValue) });
  }
}
```


## Connecting to a Server

The client **MUST** wait for the phone to signal PebbleKit JS is ready with the
'ready' event. To accomodate for this, prepare PGE WS in app initialization,
supplying the URL, and two handlers for when the connection attempt returns, and
when data is received.

When the 'ready' event is received, the connection will be attempted and the
result passed to the `PGEWSConnectedHandler`. At this point, each successfully
connected client is given an ID by the server, and can be read with
`pge_ws_get_client_id()`:

```
static void ws_connection_handler(bool successful) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Got ID: %d", pge_ws_get_client_id());
}

static void ws_received_handler() {

}

void pge_init() {
  // Start engine
  pge_begin(logic, draw, click);

  // Prepare WS for web communication with local server
  pge_ws_begin("ws://192.168.1.4:5000", ws_connection_handler, ws_received_handler);
}
```


## Disconnecting From a Server

Currently, any `AppMessage`s sent during app deinitialization (when the Back
button causes it to exit) are not successfully sent to the server. Therefore,
every client will be abandoned by the server after `EXPIRATION_MS` milliseconds
(default value is 10 minutes). Of course, WebSockets can be manually closed at
any time, and an app exiting on a real watch will trigger the connection to
close automatically (tested only on Android 4.4.5).


## Receiving Data

Read data with `pge_ws_get_value()` inside a `PGEWSReceivedHandler`
implementation, which is called everytime a packet arrives with at least one
`Tuple` in the pre-declared `AppMessage` key range (from `0` to
`PGE_WS_NUM_KEYS`). This mechanism is designed this way so that multiple key
values can be retrieved at once, useful for retrieving points or vectors. For
example:

```
// Attempt to read a value
int key_0_value = pge_ws_get_value(PGE_WS_KEY_0);

// Check this packet contains what we want
if(key_0_value != PGE_WS_NOT_FOUND) {
  // Use the value!
  APP_LOG(APP_LOG_LEVEL_INFO, "Got PGE_WS_KEY_0 value: %d", key_0_value);
}
```

The alternative paradigm is passing every received value into a callback. This
does not allow the order of callbacks to be easily determined, and would make
reading multiple values from one packet more difficult.


## Sending Data

To send data from the C side, begin a packet, add values, then send it:

```
// If connected to server
if(pge_ws_is_connected()) {
  // If beginning a packet was successful
  if(pge_ws_packet_begin()) {
    // Add some data
    pge_ws_add_int(PGE_WS_KEY_0, 42);

    // Send the data!
    pge_ws_packet_send();
  }
}
```

These values are then automatically sent to the server in
`PGEWSForwardToServer()`.


## Implementing a JS client

Use the four functions `onSocketOpen()`, `onSocketClose()`, `onSocketMessage()`,
and `onSocketError()` to implement client logic in `pebble-js-app.js`. Any data
received from the C side is automatically forwarded to the server in
`PGEWSForwardToServer()`.

Manual messages can be sent to the server using the global `webSocket` (only
available after `connectToServer()` is called. For example, using the first
three pre-defined keys to send three values, such as an id, x, and y coordinates
to the server:

```
var id = dict['PGE_WS_KEY_0'];
var x = dict['PGE_WS_KEY_1'];
var y = dict['PGE_WS_KEY_2'];

// Send position
var dataString = JSON.stringify({
  'PGE_WS_KEY_0': id,
  'PGE_WS_KEY_1': x,
  'PGE_WS_KEY_2': y
});

webSocket.send(dataString);
```

On the server side, this can be decoded in `onClientMessage()`:

```
function onClientMessage(socket, data) {
  var json = JSON.parse(data);

  // Recover values
  var id = json['PGE_WS_KEY_0'];
  var x = json['PGE_WS_KEY_1'];
  var y = json['PGE_WS_KEY_1'];
}
```