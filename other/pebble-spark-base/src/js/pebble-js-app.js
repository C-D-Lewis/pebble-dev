/*
 * Pebble to Spark base project.
 * @Chris_DL
 *
 * Instructions:
 * Fill in your Core's device ID and access token to use. Passes button press 
 * events from src/pebble-spark-base.c Clicks.
 */

// Configuration
var SHOW_LOGS = true;
var ACCESS_TOKEN = '';
var DEVICE_ID = '';

/******************************** Web *****************************************/

var importjQuery = function() {
  var script = document.createElement('script');
  script.src = 'http://code.jquery.com/jquery-latest.min.js';
  script.type = 'text/javascript';
  document.getElementsByTagName('head')[0].appendChild(script);
};

/**************************** Pebble helpers **********************************/

var hasKey = function(dict, key) {
  return typeof dict.payload[key] !== 'undefined';
};

/************************************ Other ***********************************/

function log(message) {
  if(SHOW_LOGS) console.log(message);
}

/********************************** AppMessage ********************************/

/**
 * functionName: Name of Spark.function() exposed in spark_src/main.ino
 */
var spark = function(functionName) {
  // Construct URL
  var url = 'https://api.spark.io/v1/devices/' + DEVICE_ID + '/' + functionName + '?access_token=' + ACCESS_TOKEN;
  log(url);

  // Make request (XHR does not appear to work with Spark Cloud)
  $.ajax({
    type: 'POST',
    url: url,
    data: {'args':'none'},
    success: function(data) {
      log('spark() successful: ' + JSON.stringify(data));

      // Tell Pebble
      Pebble.sendAppMessage({'KEY_PING': '1'});
    },
    error: function(e) {
      log('Failed: ' + JSON.stringify(e));

      Pebble.sendAppMessage({'KEY_PING': '0'});
    },
    dataType: 'json'
  });
}

/*********************************** Spark ************************************/

Pebble.addEventListener('ready', 
  function(e) {
    importjQuery();
    log('PebbleKit JS ready!');
  }
);

Pebble.addEventListener('appmessage',
  function(dict) {
    log('AppMessage received');

    if(hasKey(dict, 'KEY_SELECT')) {
      //Spark forward
      spark('select');
    }

    if(hasKey(dict, 'KEY_DOWN')) {
      //Spark right
      spark('down');
    }

    if(hasKey(dict, 'KEY_LONG_SELECT')) {
      //Spark back
      spark('longselect');
    }

    if(hasKey(dict, 'KEY_UP')) {
      //Spark left
      spark('up');
    }
  }
); 