// Require
var WebSocketServer = require('ws').Server;

// Config
var DEBUG = true;
var VERBOSE = true;
var PORT = 5500;
var EXPIRATION_MS = 1000 * 60 * 10;  // 10 minutes

/********************************* Helper *************************************/

function logVerbose(message) {
  if(VERBOSE) console.log(message);
}

function logDebug(message) {
  if(DEBUG) console.log(message); 
}

function log(message) {
  console.log(message);
}

/********************************* Server *************************************/

var server;
var clients = [];

var clientFromSocket = function(socket) {
  for(var i = 0; i < clients.length; i += 1) {
    if(clients[i].socket == socket) {
      return clients[i];
    }
  }
  return null;
};

function handleTimedOut(client) {
  // Remove timed out client
  var index = clients.indexOf(client);
  if(index > -1) {
    log('Removing inactive client ' + client.id);
    client.socket.close();
    clearTimeout(client.timeoutId);
    clients.splice(index, 1);
    logVerbose('Total clients: ' + clients.length);
  } else {
    logVerbose('Expired client not found!');
  }
}

function refreshClientTimeout(client) {
  logVerbose('Refreshing client ' + client.id);
  clearTimeout(client.timeoutId);

  // Set a new one
  var timeoutId = setTimeout(handleTimedOut, EXPIRATION_MS, client);
  client.timeoutId = timeoutId;
}

function addClient(socket) {
  // Create new client object
  var client = { 
    'id': Math.floor(Math.random() * 1000000),
    'socket': socket
  };
  // Set timeout ID
  var timeoutId = setTimeout(handleTimedOut, EXPIRATION_MS, client);
  client.timeoutId = timeoutId;
  clients.push(client);

  // Send back issued ID
  socket.send(JSON.stringify({ 'id': client.id }));
  log('Client ' + client.id + ' connected. Total clients: ' + clients.length);
}

function handleProtocol(socket, data) {
  var json = JSON.parse(data);
  logVerbose('Handing protocol');

  // Refresh this client
  refreshClientTimeout(clientFromSocket(socket));
}

function startServer() {
  server = new WebSocketServer({ 'port': PORT });
  server.on('connection', function (socket) {
    addClient(socket);
    onClientConnected(socket);
    socket.on('message', function(data) {
      logVerbose('onmessage');
      if(data) {
        handleProtocol(this, data);
        onClientMessage(this, data);
      }
    });
    socket.on('error', function() {
      logVerbose('onerror');
    });
    socket.on('close', function() {
      logVerbose('onclose');

      // Find client
      var client = clientFromSocket(this);
      log('Client ' + client.id + ' disconnected.');
      handleTimedOut(client);
      onClientDisconnected(client);
    });
  });
  onStartServer();
  log('Server ready on port ' + PORT);
}
startServer();

/**************************** Developer Implementation ************************/

/**
 * When the server starts
 */
function onStartServer() {

}

/**
 * Set up new client
 * socket - The web socket this client connected on
 */
function onClientConnected(socket) {

}

/**
 * Client has disconnected or timed out
 * client - The disconnected client object
 */
function onClientDisconnected(client) {

}

/**
 * React to message from client
 * socket - Web socket this message came from. Use to reply.
 * data - The data sent from the client in JSON string format
 */
function onClientMessage(socket, data) {
  
}