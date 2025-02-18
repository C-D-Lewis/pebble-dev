var MAX_ITEMS = 30;
var VERSION = "1.3.0";

/**************** JS Helpers *******************/

var persistWrite = function(key, value) {
  window.localStorage.setItem(key, value);
};

var persistRead = function(key, defaultValue) {
  if(window.localStorage.getItem(key) === null) {
    return defaultValue;
  } else {
    return window.localStorage.getItem(key);
  }
};

/*************** Requests **********************/

var xhrAsyncRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload =  function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

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

/******************* Transfer *********************/

var stories;
var current_title = 0;
var current_desc = 0;
var quantity = 0;

var sendNextDescription = function() {
  Pebble.sendAppMessage(
    {"KEY_DESCRIPTION": stories[current_desc].description},
    function(e) {
      current_desc++;

      sendNextAlternating();
    },
    function(e) {
      console.log("Sending of description " + current_desc + " failed!");
    }
  );
};

var sendNextTitle = function() {
  Pebble.sendAppMessage(
    {"KEY_TITLE": stories[current_title].title},
    function(e) {
      current_title++;

      sendNextAlternating();
    },
    function(e) {
      console.log("Sending of title " + current_title + " failed!");
    }
  );
};

/** Self limiting **/
var sendNextAlternating = function() {
  if(current_desc != (quantity + 1) && current_title != (quantity + 1)) {
    //The last incremented was desc, so desc
    if(current_title == current_desc) {
      sendNextTitle();
    } else {
      sendNextDescription();
    }
  } else {
    console.log("Upload complete!");
  }
};

/********************* Data *********************/

function Story(title, description) {
  this.description = description;
  this.title = title;
};

var parseFeed = function(text) {
  var items = [];
  while(text.indexOf("<title>") > 0) {
    //Title
    var title = text.substring(text.indexOf("<title>") + "<title>".length);
    title = title.substring(0, title.indexOf("</title>"));
    text = text.substring(text.indexOf("</title>") + "</title>".length);

    //Desc
    var desc = text.substring(text.indexOf("<description>") + "<description>".length);
    desc = desc.substring(0, desc.indexOf("</description>"));

    //Add
    items.push(new Story(title, desc));

    text = text.substring(text.indexOf("</description>") + "</description>".length);
  }

  console.log("parseFeed(): Extracted " + items.length + " items.");
  return items;
};

var download = function(category) {
  //Choose URL based on category choice
  var url;
  if(category == "headlines") {
    url = "http://feeds.bbci.co.uk/news/rss.xml";
    console.log("Selected headlines");
  } else if(category == "world") {
    url = "http://feeds.bbci.co.uk/news/world/rss.xml";
    console.log("Selected world");
  } else if(category == "uk") {
    url = "http://feeds.bbci.co.uk/news/uk/rss.xml";
    console.log("Selected uk");
  } else if(category == "politics") {
    url = "http://feeds.bbci.co.uk/news/politics/rss.xml";
    console.log("Selected politics");
  } else if(category == "health") {
    url = "http://feeds.bbci.co.uk/news/health/rss.xml";
    console.log("Selected health");
  } else if(category == "education") {
    url = "http://feeds.bbci.co.uk/news/education/rss.xml";
    console.log("Selected education");
  } else if(category == "scienv") {
    url = "http://feeds.bbci.co.uk/news/science_and_environment/rss.xml";
    console.log("Selected scienv");
  } else if(category == "technology") {
    url = "http://feeds.bbci.co.uk/news/technology/rss.xml";
    console.log("Selected technology");
  } else if(category == "entertainarts") {
    url = "http://feeds.bbci.co.uk/news/entertainment_and_arts/rss.xml";
    console.log("Selected entertainarts");
  } else {
    //Default should be supplied, but just in case
    url = "http://feeds.bbci.co.uk/news/rss.xml";
    console.log("Selected else");
  }

  //Fetch
  xhrAsyncRequest(url, "GET", 
    function(text) {
      console.log("Response from BBC obtained!");

      //Strip metadata
      text = text.substring(text.indexOf("<item>") + "<item>".length);
      stories = parseFeed(text);

      //User pref else dict length if less than max
      quantity = parseInt(persistRead("quantity", "10"));
      console.log("persistRead(): quantity read as " + quantity);
      
      //There are more than MAX
      if(quantity > MAX_ITEMS) {
        console.log("quantity > MAX_ITEMS, now " + MAX_ITEMS);
        quantity = MAX_ITEMS;
      }
        
      //There are not enough
      if(quantity > stories.length) {
        console.log("quantity > stories.length, now " + stories.length);
        quantity = stories.length;
      }

      //Start download
      Pebble.sendAppMessage(
        {"KEY_QUANTITY": quantity},
        function(e) {
          console.log("Quantity " + quantity + " sent, beginning download.");
          sendNextAlternating();
        },
        function(e) {
          console.log("Sending of quantity failed!");
        }
      );
    }
  );
  console.log("XHR request sent");
};

/***************** App Events ********************/

function reload() {
  current_index = 0;

  //Get onjects containing title, description for fast transmission
  var category = persistRead("category", "headlines");
  stories = download(category);
};

Pebble.addEventListener("ready",
  function(e) {
    console.log("PebbleKit JS ready!");
    reload();
  }
);

Pebble.addEventListener("showConfiguration",
  function(e) {
    //Load the remote config page
    Pebble.openURL("https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/bbcnews-config.html?version=" + VERSION);
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(configuration));

    //Save to localStorage
    persistWrite("category", configuration.category);
    persistWrite("quantity", configuration.quantity);

    //Send to watch
    Pebble.sendAppMessage(
      {"KEY_DETAIL_SIZE": configuration.detail_size}
    );
  }
);