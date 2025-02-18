/**
 * JS end for Pebblejs-spark-base
 * @Chris_DL
 */

var UI = require('ui');
var ajax = require('ajax');

// Put your Spark Core credentials here. Get from spark.io/build
var ACCESS_TOKEN = '';
var DEVICE_ID = '';

// Show a Card
var main = new UI.Card({
  title:'Action',
  body:'Press Up, Down or Select'
});
main.show();

/**
 * Call a function named by Spark.function()
 */
function spark(functionName) {
  var url = 'https://api.spark.io/v1/devices/' + DEVICE_ID + '/' + functionName + '?access_token=' + ACCESS_TOKEN;
  console.log('Sending Spark request: \n' + url);
  
  ajax({
    url:url,
    type:'json',
    method:'post'
  }, function(data) {
    console.log('Spark request successful: \n\n' + JSON.stringify(data));
  }, function(error) {
    console.log('Spark request FAILED: \n\n' + JSON.stringify(error));
  });
}

// Button handlers for Pebble Click actions
main.on('click', 'up', function(e) {
  spark('up');
});

main.on('click', 'select', function(e) {
  spark('select');
});

main.on('click', 'down', function(e) {
  spark('down');
});
