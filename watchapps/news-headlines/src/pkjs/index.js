var DEBUG = false;     // Turn off for release
var VERSION = '4.8';   // Match package.json
var MAX_ITEMS = 20;    // Max feed items the app will display

/*********************************** Enums ************************************/

var AppKey = {
  Title: 0,               // Story title
  Description: 1,         // Story description
  Quantity: 2,            // Total number of stories
  Index: 3,               // Which story this is
  Ready: 4,               // JS is ready
  SettingsCategory: 5,    // Selected category
  SettingsNumStories: 6,  // Number of stories to show
  SettingsRegion: 7       // Selected region
};

var Region = {
  UK: 0,
  Africa: 1,
  Asia: 2,
  Europe: 3,
  LatinAmerica: 4,
  MiddleEast: 5,
  USAndCanada: 6,
  England: 7,
  NorthernIreland: 8,
  Scotland: 9,
  Wales: 10
};

/********************************** Helpers ***********************************/

function verbose(message) {
  console.log(message);
}

function debug(message) {
  if (DEBUG) verbose(message);
}

/******************************* Requests *************************************/

function request(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}

/******************************** Pebble helpers ******************************/

var hasKey = function(dict, key) {
  return typeof dict.payload[key] !== 'undefined';
};

var getValue = function(dict, key) {
  if (hasKey(dict, key)) {
    return '' + dict.payload[key];
  } else {
    verbose('getValue(): Key ' + key + ' does not exist in received dictionary!');
    return undefined;
  }
};

var getInt = function(dict, key) {
  return parseInt(getValue(dict, key));
};

/******************************** BBC News API ********************************/

var gStories = [];
var gLastIndex = 0;
var gQuantity = 0;
var gCategory = 0;
var gRegion = 0;

var gLastOffset = 0;

var decode = function(str) {
  str = str.replace(/&amp;/g, '&');
  str = str.replace('<![CDATA[', '');
  str = str.replace(']]>', '');
  return str;
};

var parseFeed = function(responseText) {
  var items = [];

  var outerSpool = responseText;

  // Strip heading data
  outerSpool = outerSpool.substring(outerSpool.indexOf('<item>'));
  debug('parseFeed(): gQuantity=' + gQuantity);
  while(outerSpool.indexOf('<title>') > 0 && items.length < gQuantity) {
    var s = {};

    // Cap
    var spool = outerSpool.substring(0, outerSpool.indexOf('</item>'));

    // Title
    var title = spool.substring(spool.indexOf('<title>') + '<title>'.length);
    title = title.substring(0, title.indexOf('</title>'));
    if (title.indexOf('VIDEO') > -1) {
      title = title.substring(7);
    }
    s.title = decode(title);

    // Desc
    var desc = spool.substring(spool.indexOf('<description>') + '<description>'.length);
    desc = desc.substring(0, desc.indexOf('</description>'));
    s.description = decode(desc);

    // Add
    debug('parseFeed(): Sizes: ' + title.length + ', ' + desc.length);
    debug(JSON.stringify(s));
    items.push(s);

    // Next
    outerSpool = outerSpool.substring(outerSpool.indexOf('</item>') + '</item>'.length);
  }

  debug('parseFeed(): Extracted ' + items.length + ' items.');
  return items;
};

var getUKRegionCategoryURL = function(category) {
  // Parse category int
  switch(category) {
    case 0: category = 'headlines'; break;
    case 1: category = 'world'; break;
    case 2: category = 'uk'; break;
    case 3: category = 'politics'; break;
    case 4: category = 'health'; break;
    case 5: category = 'education'; break;
    case 6: category = 'science_and_environment'; break;
    case 7: category = 'technology'; break;
    case 8: category = 'entertainment_and_arts'; break;
    default: 
      debug('Defaulting to headlines for category: ' + category);
      category = 'headlines'; 
      break;
  }

  // Choose URL based on category choice
  var url;
  if (category == 'headlines') {
    url = 'http://feeds.bbci.co.uk/news/rss.xml';
  } else {
    url = 'http://feeds.bbci.co.uk/news/' + category + '/rss.xml';
  }
  debug('download(): Category: ' + category);
  return url;
};

function getURL() {
  var url;
  switch(gRegion) {
    case Region.Africa: url = 'http://feeds.bbci.co.uk/news/world/africa/rss.xml'; break;
    case Region.Asia: url = 'http://feeds.bbci.co.uk/news/world/asia/rss.xml'; break;
    case Region.Europe: url = 'http://feeds.bbci.co.uk/news/world/europe/rss.xml'; break;
    case Region.LatinAmerica: url = 'http://feeds.bbci.co.uk/news/world/latin_america/rss.xml'; break;
    case Region.MiddleEast: url = 'http://feeds.bbci.co.uk/news/world/middle_east/rss.xml'; break;
    case Region.USAndCanada: url = 'http://feeds.bbci.co.uk/news/world/us_and_canada/rss.xml'; break;
    case Region.England: url = 'http://feeds.bbci.co.uk/news/england/rss.xml'; break;
    case Region.NorthernIreland: url = 'http://feeds.bbci.co.uk/news/northern_ireland/rss.xml'; break;
    case Region.Scotland: url = 'http://feeds.bbci.co.uk/news/scotland/rss.xml'; break;
    case Region.Wales: url = 'http://feeds.bbci.co.uk/news/wales/rss.xml'; break;
    default:
      debug('Region is invalid or Region.UK (' + gRegion + '), defaulting to category ' + gCategory); 
      url = getUKRegionCategoryURL(gCategory);
      break;
  }
  return url;
}

// DO NOT modify signature - also used for images etc
function download(category, callback) {
  var url = getURL();
  request(url, 'GET', callback);
  debug('request(): request sent to ' + url);
}

/********************************** App Transfer ******************************/

function sendToWatch(responseText) {
  // User pref else dict length if less than max
  debug('sendToWatch(): quantity read as ' + gQuantity);

  // Strip metadata
  responseText = responseText.substring(responseText.indexOf('<item>') + '<item>'.length);
  gStories = parseFeed(responseText);

  // There are more than MAX
  if (gQuantity > MAX_ITEMS) {
    debug('sendToWatch(): quantity > MAX_ITEMS, now ' + MAX_ITEMS);
    gQuantity = MAX_ITEMS;
  }

  // There are not enough
  if (gQuantity > gStories.length) {
    debug('sendToWatch(): gQuantity > gStories.length, now ' + gStories.length);
    gQuantity = gStories.length;
  }

  // Start download
  var dict = {};
  dict[AppKey.Quantity] = gQuantity;
  Pebble.sendAppMessage(dict, function(e) {
    debug('sendToWatch(): Quantity ' + gQuantity + ' sent, beginning download.');
    gLastIndex = 0;
    sendNext();
  }, function(e) {
    debug('sendToWatch(): Sending of gQuantity failed!');
  });
}

// Upload one story at a time
function sendNext() {
  if (gLastIndex < gQuantity) {
    var dict = {};
    dict[AppKey.Index] = gLastIndex;
    dict[AppKey.Title] = gStories[gLastIndex].title;
    dict[AppKey.Description] = gStories[gLastIndex].description;
    Pebble.sendAppMessage(dict, function() {
      debug('sendNext(): Sent story ' + gLastIndex);
      sendNext();
    }, function(err) {
      verbose('sendNext(): Error sending story ' + gLastIndex + ': ' + err);
    });

    gLastIndex += 1;
  } else {
    verbose('sendNext(): Sent all stories to Pebble!');
  }
}

/********************************** PebbleKit JS ******************************/

Pebble.addEventListener('ready', function(e) {
  verbose('ready: PebbleKit JS ready! Version ' + VERSION);

  var dict = {};
  dict[AppKey.Ready] = 1;
  debug('dict: ' + JSON.stringify(dict));
  Pebble.sendAppMessage(dict, function() {
    debug('ready: Sent ready to watchapp.');
  }, function(error) {
    verbose('ready: Failed to send ready to watchapp: ' + error.message);
  });
});

Pebble.addEventListener('appmessage', function(dict) {
  debug('appmessage: ' + JSON.stringify(dict.payload));

  // Settings - if category exists, the others will too
  if (hasKey(dict, AppKey.SettingsCategory)) {
    gCategory = getInt(dict, AppKey.SettingsCategory);
    gQuantity = getInt(dict, AppKey.SettingsNumStories);
    gRegion = getInt(dict, AppKey.SettingsRegion);

    debug('appmessage: Watch sent settings: ' + gCategory + '/' + gQuantity + '/' + gRegion);
    download(gCategory, sendToWatch);
  }
});
