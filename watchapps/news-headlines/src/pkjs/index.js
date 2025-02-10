var config = require('./config.json');

var DEBUG = false;  // Turn off for release
var VERSION = '4.6';  // Match package.json
var MAX_ITEMS = 20;   // Max feed items the app will display
var DATA_SIZE = 1950; // Max AppMessage image chunk size (pre-dates 8k buffers)
var THUMBNAIL_WIDTH = 144;
var THUMBNAIL_HEIGHT = 81;

var TOPIC_HEADLINES = 'headlines';
var TOPIC_HEADLINES_NOTIFS = 'headlines-notifs';

/*********************************** Enums ************************************/

var ServerStatus = {
  Waiting: 0,
  Up: 1,
  Down: 2,
  Timeout: 3
};

var AppKey = {
  Title: 0,                    // Story title
  Description: 1,              // Story description
  Quantity: 2,                 // Total number of stories
  Index: 3,                    // Which story this is
  Failed: 5,                   // The pin story was not found
  Status: 9,                   // Query pin server status
  Ready: 10,                   // JS is ready
  ImageFailed: 11,             // Failed to download image
  Img: 12,                     // Fetch an image
  Offset: 13,                  // Offset in an image
  Data: 14,                    // Actual image data
  ImageDone: 15,               // Image should be complete
  ChunkSize: 16,               // Size of incoming image chunk
  ImageAvailabilityString: 17  // String of '0' and '1' representing which stories have images
};

var AppKeySettings = {
  Category: 6,
  Subscription: 7,
  NumStories: 8,
  Region: 9
};

var PinSubscriptionType = {
  NotSubscribed: 0,
  Subscribed: 1
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
  if(DEBUG) verbose(message);
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
  if(hasKey(dict, key)) {
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

var gLaunchCode = 0;
var gLastOffset = 0;
var gImgSession = 0;

var decode = function(str) {
  str = str.replace(/&amp;/g, '&');
  str = str.replace('<![CDATA[', '');
  str = str.replace(']]>', '');
  return str;
};

var parseFeed = function(responseText) {
  var items = [];
  var longestTitle = 0;
  var longestDesc = 0;

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
    if(title.indexOf('VIDEO') > -1) {
      title = title.substring(7);
    }
    s.title = decode(title);

    // Desc
    var desc = spool.substring(spool.indexOf('<description>') + '<description>'.length);
    desc = desc.substring(0, desc.indexOf('</description>'));
    s.description = decode(desc);

    // Image URL - NO LONGER 144PX WIDE :( (June 2016)
    var urlIndex = spool.indexOf('<media:thumbnail width="144"');
    if(urlIndex > -1) {
      var url = spool.substring(urlIndex + '<media:thumbnail width="144"'.length);
      url = url.substring(url.indexOf('url=') + 'url='.length + 1);
      s.url = url.substring(0, url.indexOf('"/>'));
    } else {
      s.url = 'NONE';
    }

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
  if(category == 'headlines') {
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

function sendNextImageChunk(arr, thisImgSession) {
  var chunkSize;
  var size = THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT;
  var last = false;
  if(gLastOffset + DATA_SIZE < size) {
    // Not near the end
    chunkSize = DATA_SIZE;
  } else {
    // Calculate last chunk size
    chunkSize = size - gLastOffset;
    last = true;
  }

  var slice = arr.slice(gLastOffset, gLastOffset + chunkSize);
  var dict = {};
  dict[AppKey.Offset] = gLastOffset;
  dict[AppKey.Data] = slice;
  dict[AppKey.ChunkSize] = chunkSize;

  if(last === true) {
    dict[AppKey.ImageDone] = 1;
  }

  if(thisImgSession == gImgSession) {
    Pebble.sendAppMessage(dict, function() {
      debug('sendNextImageChunk(): Sent chunk: ' + gLastOffset);
      gLastOffset += chunkSize;
      if(last === false) {
        sendNextImageChunk(arr, thisImgSession);
      }
    }, function(error) {
      verbose('sendNextImageChunk(): Error sending image chunk: ' + JSON.stringify(error));
    });
  } else {
    verbose('sendNextImageChunk(): Session ID has changed, aborting this transfer: ' + thisImgSession);
  }
}

function downloadImage(responseText, titleHash, thisImgSession) {
  var spool = responseText.substring(responseText.indexOf('<item>') + '<item>'.length);
  gQuantity = MAX_ITEMS; // Get all
  var stories = parseFeed(spool);

  // Test
  var index = -1;
  if(titleHash < 0) {
    // Just pick the first
    index = 0;
  } else {
    // Find match
    var found = false;
    for(var i = 0; i < stories.length; i += 1) {
      var check = checksum(stories[i].title);
      if(('' + check == '' + titleHash) && (stories[i].url !== 'NONE')) {
        debug('downloadImage(): Found! check=' + check + ', titleHash=' + titleHash);
        index = parseInt(i);
        found = true;
        break;
      }
    }
    if(found === false) {
      var dict = {};
      dict[AppKey.ImageFailed] = 1;
      Pebble.sendAppMessage(dict, function() {
        debug('downloadImage(): Informed Pebble of failure to find story.');
      }, function(err) {
        debug('downloadImage(): Failed to inform of failure!');
      });
      return;
    }
  }

  if(index >= 0) {
    // Story was found, get boot url
    request(config.BOOT_URL, 'GET', function(responseText) {
      var json;
      try {
        json = JSON.parse(responseText);
      } catch(err) {
        debug('downloadImage(): Error parsing response: ' + err.message | err);
        var dict = {};
        dict[AppKey.ImageFailed] = 1;
        Pebble.sendAppMessage(dict, function() {
          debug('downloadImage(): Informed Pebble of failure to find story.');
        }, function(err) {
          debug('downloadImage(): Failed to inform of failure!');
        });
      }

      if(!json) {
        debug('downloadImage(): Failed to get JSON, returning early.');
        return;
      }

      // Get image data
      var ip = json.ip;
      var reqUrl = 'http://' + ip + ':5000/convert?url=' + stories[index].url;
      debug('downloadImage(): Requesting image for ' + stories[i].url);
      request(reqUrl, 'GET', function(responseText) {
        debug('downloadImage(): Got conversion response!');

        var values = [];
        var spool = responseText.substring(responseText.indexOf(1));
        while(spool.indexOf(',') > 0) {
          values.push(parseInt(spool.substring(0, spool.indexOf(','))));
          spool = spool.substring(spool.indexOf(',') + 1);
        }
        // Get last value
        values.push(parseInt(spool.substring(0, spool.indexOf(']'))));
        verbose('downloadImage(): Parsed ' + values.length + ' values. (' + (144 * 81) + ' expected)');

        var result = [];
        var uint_arr = new Uint8Array(values);
        for(var i = 0; i < uint_arr.byteLength; i++) {
          result.push(uint_arr[i]);
        }

        gLastOffset = 0;
        if(thisImgSession == gImgSession) {
          sendNextImageChunk(result, thisImgSession);
        }
      }, function(error) {
        verbose('downloadImage(): Error getting image data: ' + error);
      });
    });
  }
}

/********************************** App Transfer ******************************/

function sendToWatch(responseText) {
  debug('sendToWatch(): Response from BBC obtained!');

  // User pref else dict length if less than max
  debug('sendToWatch(): quantity read as ' + gQuantity);

  // Strip metadata
  responseText = responseText.substring(responseText.indexOf('<item>') + '<item>'.length);
  gStories = parseFeed(responseText);

  // Get image availability
  var imagesAvailable = '';
  for(var i = 0; i < gStories.length; i += 1) {
    if(gStories[i].url !== 'NONE') {
      imagesAvailable += '1';
    } else {
      imagesAvailable += '0';
    }
  }
  debug('imagesAvailable: ' + imagesAvailable);

  // There are more than MAX
  if(gQuantity > MAX_ITEMS) {
    debug('sendToWatch(): quantity > MAX_ITEMS, now ' + MAX_ITEMS);
    gQuantity = MAX_ITEMS;
  }

  // There are not enough
  if(gQuantity > gStories.length) {
    debug('sendToWatch(): gQuantity > gStories.length, now ' + gStories.length);
    gQuantity = gStories.length;
  }

  // Start download
  var dict = {};
  dict[AppKey.Quantity] = gQuantity;
  dict[AppKey.ImageAvailabilityString] = imagesAvailable;
  Pebble.sendAppMessage(dict, function(e) {
    debug('sendToWatch(): Quantity ' + gQuantity + ' sent, beginning download.');
    gLastIndex = 0;
    sendNext();
  }, function(e) {
    debug('sendToWatch(): Sending of gQuantity failed!');
  });
}

// function findPinWithHash(responseText) {
//   // Strip metadata
//   var spool = responseText.substring(responseText.indexOf('<item>') + '<item>'.length);
//   gQuantity = MAX_ITEMS; // Get all
//   var stories = parseFeed(spool);

//   debug('findPinWithHash(): Finding title with launchCode=' + gLaunchCode + ' in list of ' + stories.length + ' stories');

//   var found = false;
//   for(var i = 0; i < stories.length; i += 1) {
//     var check = checksum(stories[i].title);
//     if('' + check == '' + gLaunchCode) {
//       debug('findPinWithHash(): Found! check=' + check + ', gLaunchCode=' + gLaunchCode);

//       // Send to watch
//       var dict = {
//         'KEY_ACTION': 0,
//         'KEY_TITLE': stories[i].title,
//         'KEY_DESCRIPTION': stories[i].description
//       };
//       Pebble.sendAppMessage(dict, function() {
//         debug('findPinWithHash(): Sent pin data to watch!');
//         found = true;
//       });
//     }
//   }

//   // Not found?
//   if(found == false) {
//     var dict = { 'KEY_FAILED': 1 };
//     Pebble.sendAppMessage(dict, function() {
//       debug('findPinWithHash(): Informed Pebble of failure to find story.');
//     }, function(err) {
//       debug('findPinWithHash(): Failed to inform of failure!');
//     });
//   }
// }

// Upload one story at a time
function sendNext() {
  if(gLastIndex < gQuantity) {
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

var checksum = function(input) {
  var result = 0;
  for(var j = 0; j < input.length; j += 1) {
    result += input.charCodeAt(j);
  }
  debug('checksum(): Generated checksum: ' + result);
  return result;
};

function getStatus() {
  // Query boot for IP
  request(config.BOOT_URL, 'GET', function(responseText) {
    var json = JSON.parse(responseText);
    var ip = json.ip;

    // Get status
    request('http://' + ip + ':5000/status', 'GET', function(responseTextStatus) {
      debug('getStatus(): Status response: ' + responseTextStatus);

      var out = {};
      if(responseTextStatus.indexOf('OK') > -1) {
        debug('getStatus(): Server is up!');
        out[AppKey.Status] = ServerStatus.Up;
      } else {
        debug('getStatus(): Server is down!');
        out[AppKey.Status] = ServerStatus.Down;
      }
      Pebble.sendAppMessage(out, function() {
        debug('getStatus(): Sent status to Pebble!');
      }, function() {
        debug('getStatus(): Failed sending status');
      });
    });
  });
}

/********************************** PebbleKit JS ******************************/

Pebble.addEventListener('ready', function(e) {
  verbose('ready: PebbleKit JS ready! Version ' + VERSION);

  var dict = {};
  dict['' + AppKey.Ready] = 1;  // Hack to use enum values in square brackets
  debug('dict: ' + JSON.stringify(dict));
  Pebble.sendAppMessage(dict, function() {
    debug('ready: Sent ready to watchapp.');
  }, function(error) {
    verbose('ready: Failed to send ready to watchapp: ' + error.message);
  });
});

Pebble.addEventListener('appmessage', function(dict) {
  debug('appmessage: ' + JSON.stringify(dict.payload));

  // Pin?
  // if(hasKey(dict, 'KEY_ACTION')) {
  //   gLaunchCode = getValue(dict, 'KEY_ACTION');
  //   debug('appmessage: TIMELINE PIN LAUNCH CODE: ' + gLaunchCode + '\n\n\n');

  //   // Find from all
  //   gQuantity = MAX_ITEMS;

  //   // Download stories, and match the titles to the pin
  //   download(gCategory, findPinWithHash);
  // }

  // Image?
  if(hasKey(dict, AppKey.Img)) {
    gCategory = getInt(dict, AppKeySettings.Category);
    var hash = getInt(dict, AppKey.Img);

    // Handle fetching an image while a download is in the air
    gImgSession = Math.round(Math.random() * 10000);
    debug('appmessage: gImgSession now: ' + gImgSession);

    // Get this image
    download(gCategory, function(responseText) {
      downloadImage(responseText, hash, gImgSession);
    });
  }

  // Settings - if category exists, the others will too
  else if(hasKey(dict, AppKeySettings.Category)) {
    gCategory = getInt(dict, AppKeySettings.Category);
    gQuantity = getInt(dict, AppKeySettings.NumStories);
    gRegion = getInt(dict, AppKeySettings.Region);

    var subscribedType = getInt(dict, AppKeySettings.Subscription);
    editPinSubscriptions(subscribedType);

    debug('appmessage: Watch sent settings: ' + gCategory + '/' + subscribedType + '/' + gQuantity + '/' + gRegion);
    download(gCategory, sendToWatch);
  }

  // Server query
  else if(hasKey(dict, AppKey.Status)) {
    getStatus();
    debug('appmessage: Getting server status...');
  }
});

function editPinSubscriptions(enumValue) {
  if(Pebble.timelineSubscribe) {
    switch(enumValue) {
      case PinSubscriptionType.NotSubscribed:
        Pebble.timelineUnsubscribe(TOPIC_HEADLINES,
          function(success) { debug('editPinSubscriptions(): Unsub from headlines OK'); },
          function(error)   { verbose('editPinSubscriptions(): Unsub error: ' + error); });
        // Pebble.timelineUnsubscribe(TOPIC_HEADLINES_NOTIFS,
        //   function(success) { debug('editPinSubscriptions(): Unsub from headlines-notifs OK'); },
        //   function(error)   { verbose('editPinSubscriptions(): Unsub error: ' + error); });
        break;
      case PinSubscriptionType.Subscribed:
        Pebble.timelineSubscribe(TOPIC_HEADLINES,
          function(success) { debug('editPinSubscriptions(): Sub to headlines OK'); },
          function(error)   { verbose('editPinSubscriptions(): Sub error: ' + error); });
        // Pebble.timelineUnsubscribe(TOPIC_HEADLINES_NOTIFS,
        //   function(success) { debug('editPinSubscriptions(): Unsub from headlines-notifs OK'); },
        //   function(error)   { verbose('editPinSubscriptions(): Unsub error: ' + error); });
        break;
      // case PinSubscriptionType.SubscribedWithNotifs:
      //   Pebble.timelineSubscribe(TOPIC_HEADLINES_NOTIFS,
      //     function(success) { debug('editPinSubscriptions(): Sub to headlines-notifs OK'); },
      //     function(error)   { verbose('editPinSubscriptions(): Sub error: ' + error); });
      //   Pebble.timelineUnsubscribe(TOPIC_HEADLINES,
      //     function(success) { debug('editPinSubscriptions(): Unsub from headlines OK'); },
      //     function(error)   { verbose('editPinSubscriptions(): Unsub error: ' + error); });
      //   break;
      default:
        verbose('editPinSubscriptions(): Unknown enumValue: ' + enumValue);
        break;
    }
  }
}
