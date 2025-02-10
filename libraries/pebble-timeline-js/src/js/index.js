// The timeline public URL root
var API_URL_ROOT = 'https://timeline-api.rebble.io/';
var TAG = 'pebble-timeline-js';

function Log(msg) {
  console.log(TAG + ': ' + msg);
}

/**
 * Send a request to the Rebble public web timeline API.
 * @param pin The JSON pin to insert. Must contain 'id' field.
 * @param type The type of request, either PUT or DELETE.
 * @param callback The callback to receive the responseText after the request has completed.
 */
function timelineRequest(pin, type, callback) {
  // User or shared?
  var url = API_URL_ROOT + 'v1/user/pins/' + pin.id;

  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    Log('response received: ' + this.responseText);
    callback(this.responseText);
  };
  xhr.open(type, url);

  // Set headers
  xhr.setRequestHeader('Content-Type', 'application/json');

  // Get token
  Pebble.getTimelineToken(function(token) {
    // Add headers
    xhr.setRequestHeader('X-User-Token', '' + token);

    // Send
    xhr.send(JSON.stringify(pin));
    Log('request sent.');
  }, function() { Log('error getting timeline token'); });
}

/**
 * Insert a pin into the timeline for this user.
 * @param pin The JSON pin to insert.
 * @param callback The callback to receive the responseText after the request has completed.
 */
function insertUserPin(pin, callback) {
  timelineRequest(pin, 'PUT', callback);
}

/**
 * Delete a pin from the timeline for this user.
 * @param pin The JSON pin to delete.
 * @param callback The callback to receive the responseText after the request has completed.
 */
function deleteUserPin(pin, callback) {
  timelineRequest(pin, 'DELETE', callback);
}

/**
* Set the user's AppGlances with an array of slice objects.
* @param slices An array of AppGlance slice objects (https://developer.rebble.io/developer.pebble.com/guides/user-interfaces/appglance-rest/#creating-slices)
* @param callback Callback called when the request is resolved.
*/
function setAppGlances(slices, callback) {
  var url = API_URL_ROOT + 'v1/user/glance/';

  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    Log('response received: ' + this.responseText);
    callback(this.responseText);
  };
  xhr.open('PUT', url);

  // Set headers
  xhr.setRequestHeader('Content-Type', 'application/json');

  // Get token
  Pebble.getTimelineToken(function(token) {
    // Add headers
    xhr.setRequestHeader('X-User-Token', '' + token);

    // Send
    xhr.send(JSON.stringify({ 'slices': slices }));
    Log('AppGlance request sent.');
  }, function() { Log('error getting timeline token'); });
}

/********************************** Exports ***********************************/

exports.insertUserPin = insertUserPin;
exports.deleteUserPin = deleteUserPin;
exports.setAppGlances = setAppGlances;
