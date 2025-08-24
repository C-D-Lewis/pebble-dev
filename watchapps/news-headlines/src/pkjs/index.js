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

/******************************* Requests *************************************/

function request(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
  console.log('request(): ' + url);
}

/******************************** Pebble helpers ******************************/

var hasKey = function(dict, key) {
  return typeof dict.payload[key] !== 'undefined';
};

var getValue = function(dict, key) {
  if (hasKey(dict, key)) {
    return '' + dict.payload[key];
  } else {
    console.log('getValue(): Key ' + key + ' does not exist in received dictionary!');
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
    // console.log(JSON.stringify(s));
    items.push(s);

    // Next
    outerSpool = outerSpool.substring(outerSpool.indexOf('</item>') + '</item>'.length);
  }

  console.log('parseFeed(): Extracted ' + items.length + ' items.');
  return items;
};

var getUKRegionCategoryURL = function(category) {
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
      console.log('Defaulting to headlines for category: ' + category);
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
  return url;
};

function getURL() {
  switch(gRegion) {
    case Region.Africa:          return 'http://feeds.bbci.co.uk/news/world/africa/rss.xml';
    case Region.Asia:            return 'http://feeds.bbci.co.uk/news/world/asia/rss.xml';
    case Region.Europe:          return 'http://feeds.bbci.co.uk/news/world/europe/rss.xml';
    case Region.LatinAmerica:    return 'http://feeds.bbci.co.uk/news/world/latin_america/rss.xml';
    case Region.MiddleEast:      return 'http://feeds.bbci.co.uk/news/world/middle_east/rss.xml';
    case Region.USAndCanada:     return 'http://feeds.bbci.co.uk/news/world/us_and_canada/rss.xml';
    case Region.England:         return 'http://feeds.bbci.co.uk/news/england/rss.xml';
    case Region.NorthernIreland: return 'http://feeds.bbci.co.uk/news/northern_ireland/rss.xml';
    case Region.Scotland:        return 'http://feeds.bbci.co.uk/news/scotland/rss.xml';
    case Region.Wales:           return 'http://feeds.bbci.co.uk/news/wales/rss.xml';
    default:
      console.log('getURL(): Region is invalid or is UK: ' + gRegion + ', defaulting to selected category ' + gCategory); 
      return getUKRegionCategoryURL(gCategory);
  }
}

/********************************** App Transfer ******************************/

function sendToWatch(responseText) {
  // User pref else dict length if less than max
  console.log('sendToWatch(): initial quantity: ' + gQuantity);

  // Strip metadata
  responseText = responseText.substring(responseText.indexOf('<item>') + '<item>'.length);
  gStories = parseFeed(responseText);

  // There are more than MAX
  if (gQuantity > MAX_ITEMS) {
    console.log('sendToWatch(): quantity > MAX_ITEMS, now ' + MAX_ITEMS);
    gQuantity = MAX_ITEMS;
  }

  // There are not enough
  if (gQuantity > gStories.length) {
    console.log('sendToWatch(): gQuantity > gStories.length, now ' + gStories.length);
    gQuantity = gStories.length;
  }

  // Start upload
  var dict = {};
  dict[AppKey.Quantity] = gQuantity;
  Pebble.sendAppMessage(dict, function(e) {
    console.log('sendToWatch(): Quantity ' + gQuantity + ' sent, beginning upload.');
    gLastIndex = 0;
    sendNextStory();
  }, function(e) {
    console.log('sendToWatch(): Sending of gQuantity failed!');
  });
}

// Upload one story at a time
function sendNextStory() {
  if (gLastIndex === gQuantity) {
    console.log('sendNextStory(): Sent all stories to Pebble!');
    return;
  }

  var dict = {};
  dict[AppKey.Index] = gLastIndex;
  dict[AppKey.Title] = gStories[gLastIndex].title;
  dict[AppKey.Description] = gStories[gLastIndex].description;

  Pebble.sendAppMessage(dict, function() {
    gLastIndex += 1;
    sendNextStory();
  }, function(err) {
    console.log('sendNextStory(): Error sending story ' + gLastIndex + ': ' + err);
  });

}

/********************************** PebbleKit JS ******************************/

Pebble.addEventListener('ready', function(e) {
  console.log('ready: PebbleKit JS ready!');

  var dict = {};
  dict[AppKey.Ready] = 1;
  Pebble.sendAppMessage(dict, function() {
    console.log('ready: Sent ready to watchapp.');
  }, function(error) {
    console.log('ready: Failed to send ready to watchapp: ' + error.message);
  });
});

Pebble.addEventListener('appmessage', function(dict) {
  console.log('appmessage: ' + JSON.stringify(dict.payload));

  // Settings - if category exists, the others will too
  if (hasKey(dict, AppKey.SettingsCategory)) {
    gCategory = getInt(dict, AppKey.SettingsCategory);
    gQuantity = getInt(dict, AppKey.SettingsNumStories);
    gRegion = getInt(dict, AppKey.SettingsRegion);
    console.log('appmessage: Watch sent settings: ' + gCategory + '/' + gQuantity + '/' + gRegion);

    request(getURL(), 'GET', sendToWatch);
  }
});
