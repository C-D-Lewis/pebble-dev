var VERSION = "1.0";

Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
  }
);

Pebble.addEventListener("showConfiguration",
  function(e) {
    //Load the remote config page
    Pebble.openURL("https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/sevensegments-config.html?version=" + VERSION);
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(configuration));

    //Send to watch
    Pebble.sendAppMessage(
      {"KEY_INVERTED": configuration.inverted},
      function(e) {
        console.log('Configuration send successful');
      },
      function(e) {
        console.log('Configuration send NOT successful');
      }
    );
  }
);