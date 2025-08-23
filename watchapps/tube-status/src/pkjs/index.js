/********************************** Helpers ***********************************/

function requestJson(url, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    callback(JSON.parse(this.responseText));
  };
  xhr.open('GET', url);
  xhr.send();
};

/************************************* App ************************************/

/**
 * Send all lines statuses to Pebble.
 *
 * @param {Object} lineStates Object mapping line IDs to status strings.
 */
function sendToPebble(lineStates) {
  // TODO: Stream messages with larger lineStatuses[n].reason string for detail window
  var dict = {
    'LineTypeBakerloo': lineStates['bakerloo'] || '?',
    'LineTypeCentral': lineStates['central'] || '?',
    'LineTypeCircle': lineStates['circle'] || '?',
    'LineTypeDistrict': lineStates['district'] || '?',
    'LineTypeDLR': lineStates['dlr'] || '?',
    'LineTypeElizabeth': lineStates['elizabeth'] || '?',
    'LineTypeHammersmithAndCity': lineStates['hammersmith-city'] || '?',
    'LineTypeJubilee': lineStates['jubilee'] || '?',
    'LineTypeLiberty': lineStates['liberty'] || '?',
    'LineTypeLioness': lineStates['lioness'] || '?',
    'LineTypeMetropolitan': lineStates['metropolitan'] || '?',
    'LineTypeMildmay': lineStates['mildmay'] || '?',
    'LineTypeNorthern': lineStates['northern'] || '?',
    'LineTypePicadilly': lineStates['piccadilly'] || '?',
    'LineTypeSuffragette': lineStates['suffragette'] || '?',
    'LineTypeVictoria': lineStates['victoria'] || '?',
    'LineTypeWaterlooAndCity': lineStates['waterloo-city'] || '?',
    'LineTypeWeaver': lineStates['weaver'] || '?',
    'LineTypeWindrush': lineStates['windrush'] || '?'
  };

  Pebble.sendAppMessage(dict, function(e) {
    console.log('Sent!');
  }, function(e) {
    console.log('Send failed!');
  });
}

/**
 * Download all lines statuses.
 *
 * @param {string[]} lineIds Array of line IDs, e.g. ['tube', 'dlr'] etc
 */
function downloadStatus(lineIds) {
  requestJson('https://api.tfl.gov.uk/line/mode/' + lineIds.join(',') + '/status', function(json) {
    var result = json.reduce(function(acc, obj) {
      acc[obj.id] = '?';
    
      if (obj.lineStatuses && obj.lineStatuses.length > 0) {
        acc[obj.id] = obj.lineStatuses[0].statusSeverityDescription;
      }
      return acc;
    }, {});
    console.log(JSON.stringify(result));

    sendToPebble(result);
  });
}

/******************************** PebbleKit JS ********************************/

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready');

  // Available modes: https://api.tfl.gov.uk/StopPoint/Meta/modes
  downloadStatus([
    'tube',
    'dlr',
    'elizabeth-line',
    'overground',
  ]);
});
