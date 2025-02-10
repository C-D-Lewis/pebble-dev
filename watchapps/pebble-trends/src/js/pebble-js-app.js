// Configuration
var DEVICE_ID = '53ff78065075535147131587';
var ACCESS_TOKEN = '4a5ad03cc220cd0e50c81dffff7fa64ed667c6b0';

// Trend global storage
var trends;
var trendIndex = 0;

/**************** Pebble helpers ****************/

var hasKey = function(dict, key) {
  return typeof dict.payload[key] !== "undefined";
};

var getValue = function(dict, key) {
  if(hasKey(dict, key)) {
    return "" + dict.payload[key];
  } else {
    console.log("getValue(): Key '" + key + "' does not exist in received dictionary!");
    return undefined;
  }
};

/**
 * Import jQuery to the document
 */
var importjQuery = function() {
    var script = document.createElement('script');
    script.src = 'http://code.jquery.com/jquery-latest.min.js';
    script.type = 'text/javascript';
    document.getElementsByTagName('head')[0].appendChild(script);
};

/**
 * Use jQuery to query the Spark API
 */
function sparkRequest(deviceId, accessToken, functionName, args, callback) {
  // Construct URL
  var URL = 'https://api.spark.io/v1/devices/' + deviceId + '/' + functionName + '?access_token=' + ACCESS_TOKEN;

  console.log('ajax: URL=' + URL);

  //Send with jQuery
  $.ajax({
    type: 'POST',
    url: URL,
    data: {
      'args': args
    },
    success: callback,
    error: function(error, e1, e2) {
      console.log('error in ajax');
    },
    dataType: 'json'
  });
}

var sendIndex = 0;

function sendNextTrend() {
  // Trim timestamp
  var stamp = trends[sendIndex].first_trended_at;
  stamp = stamp.substring(stamp.indexOf('T') + 1, stamp.indexOf('+'));

  var dict = {
    "KEY_TITLE": trends[sendIndex].name,
    "KEY_TIMESTAMP": stamp
  };

  // Check for non ascii chars
  var regex = /^[ -~]+$/;
  if(regex.test(trends[sendIndex].name)) {
    Pebble.sendAppMessage(dict,
      function(e) {
        // Only increase if valid, prevent array bubbles
        sendIndex += 1;
        
        if(sendIndex < trends.length) {
          sendNextTrend();
        } else {
          console.log('Download complete!');
          Pebble.sendAppMessage({'KEY_FINISHED': 0});
        }
      },
      function(e) {
        console.log('Error in sendNextTrend()');
      }
    );
  } else {
    console.log('Trend ' + sendIndex + ' contains non ascii characters, splicing...');

    // Remove offending trend
    trends.splice(sendIndex, 1);

    // Skip it
    // sendIndex += 1;
    sendNextTrend();
  }
}

/**
 * When the trends array has been written
 */
function getTrendsSuccess() {
  // Print all trends
  console.log();
  for(var i = 0; i < trends.length; i++) {
    console.log('Trend #' + i + ': ' + trends[i].name);
  }
  
  // Start sending to Pebble
  sendNextTrend();
}

/**
 * Download the trends and execute getTrendsSuccess if that
 */
function getTrends() {
  // Get the JSON
  $.ajax({
    type: 'GET',
    url: 'http://api.whatthetrend.com/api/v2/trends.json',
    success: function(response) {
      // Save the results
      trends = response.trends;
      getTrendsSuccess();
    },
    error: function(error, e1, e2) {
      console.log('error in ajax');
    },
    dataType: 'json'
  });
}

/****************************** Pebble App Events ****************************/

Pebble.addEventListener('ready',
  function(e) {
    // Import jQuery
    importjQuery();
    
    console.log('PebbleKit JS ready!');
  }
);

Pebble.addEventListener('appmessage',
  function(dict) {
    console.log('AppMessage received!');

    if(hasKey(dict, 'KEY_TITLE')) {
      console.log('Requesting trends...');

      // Request?
      getTrends();
    }

    else if(hasKey(dict, 'KEY_SPARK_SHOW')) {
      console.log('Sending to Spark...');

      var which = getValue(dict, 'KEY_SPARK_SHOW');

      sparkRequest(DEVICE_ID, ACCESS_TOKEN, 'set', trends[which].name, 
        function(responseJSON) {
          console.log('Sent #' + which + ' to Spark LCD');
        }
      );
    }
  }
);