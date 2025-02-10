var VERSION = '1.2';

// https://github.com/smallstoneapps/gcolor.js/blob/master/lib/gcolor.js
function GColorFromHex(hex) {
  var hexNum = parseInt(hex, 16);
  var a = 192;
  var r = (((hexNum >> 16) & 0xFF) >> 6) << 4;
  var g = (((hexNum >>  8) & 0xFF) >> 6) << 2;
  var b = (((hexNum >>  0) & 0xFF) >> 6) << 0;
  return a + r + g + b;
}

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready! Version ' + VERSION);
});

Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL('https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/brackets-config.html?version=' + VERSION);
});

var toBoolInt = function(str) {
  return (str === 'true') ? 1 : 0;
}

Pebble.addEventListener('webviewclosed', function(e) {
  var json = JSON.parse(decodeURIComponent(e.response));

  Pebble.sendAppMessage({
    'AppKeyBatteryMeter': '' + json.check_battery_meter,
    'AppKeyBluetoothAlert': '' + json.check_bluetooth_alerts,
    'AppKeyDashedLine': '' + json.check_dashed_line,
    'AppKeySecondTick': '' + json.check_second_tick,

    'AppKeyBackgroundColor': GColorFromHex(json.text_background_color),
    'AppKeyDateColor': GColorFromHex(json.text_date_color),
    'AppKeyTimeColor': GColorFromHex(json.text_time_color),
    'AppKeyLineColor': GColorFromHex(json.text_line_color),
    'AppKeyBracketColor': GColorFromHex(json.text_bracket_color),
    'AppKeyComplicationColor': GColorFromHex(json.text_complication_color)
  }, function() {
    console.log('Settings update successful!');
  }, function(e) {
    console.log('Settings update failed: ' + JSON.stringify(e) + " for update " + JSON.stringify(options));
  });
});
