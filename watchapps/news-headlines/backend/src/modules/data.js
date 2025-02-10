const request = require('request');
const moment = require('moment-timezone');
const timelinejs = require('pebble-timeline-js-node');

const config = require('../node-common').config();
const log = require('../node-common').log();
const fcm = require('../node-common').fcm();
const extract = require('../node-common').extract();

config.requireKeys('data.js', {
  required: [ 'ENV' ],
  type: 'object', properties: {
    ENV: {
      required: [ 'API_KEY_PROD', 'API_KEY_SANDBOX' ],
      type: 'object', properties: {
        API_KEY_PROD: { type: 'string' },
        API_KEY_SANDBOX: { type: 'string' }
      }
    }
  }
});

const MAX_PUSHED = 1;  // Max pins pushed each INTERVAL. Prevents infamous timeline blob db errors.
const MAX_DUPLICATES = 50;  // Max 'already pushed' stories

var dupeBuffer = [];  // Check new stories against the last MAX_DUPLICATES to prevent hour-later duplucates
var cacheFirst = true;  // Don't post pins right away - stateful

function decode(str) {
  str = str.split(/&amp;/g).join('&');
  str = str.split('<![CDATA[').join('');
  return str.split(']]>').join('');
}

function getStories(xml) {
  const items = [];
  xml = xml.split('<item>');
  xml.shift();
  xml.map((xmlChunk) => {
    const story = {
      title: decode(extract(xmlChunk, [ '<title>' ], '</title>')),
      description: decode(extract(xmlChunk, [ '<description>' ], '</description>')),
      date: extract(xmlChunk, [ '<pubDate>' ], '</pubDate>')
    };

    if(!dupeBuffer.find((dupe) => dupe.title === story.title)) {
      dupeBuffer.unshift(story);
      if(dupeBuffer.length > MAX_DUPLICATES) dupeBuffer.pop();
     
      items.push(story);
      log.debug(`Added new story: \n${story.title}\n${story.description}\n`);
    }
  });

  log.debug(`There are ${dupeBuffer.length} items in the duplicate buffer`);
  log.info(`Extracted ${items.length} items.`);
  return items;
};

function pushPin(stories, index) {
  const pubDate = moment(stories[index].date);
  var pin = {
    id: 'bbcnews-story-' + pubDate.unix(),
    time: pubDate.toDate(),
    layout: {
      type: 'genericPin',
      tinyIcon: 'system://images/NEWS_EVENT',
      title: stories[index].title,
      subtitle: 'BBC News',
      body: stories[index].description,
      foregroundColor: '#FFFFFF',
      backgroundColor: '#AA0000'
    }
  };
  log.debug(`pin=${JSON.stringify(pin)}`);

  const TOPIC = 'headlines';
  timelinejs.insertSharedPin(pin, [ TOPIC ], config.ENV.API_KEY_PROD, log.info);
  timelinejs.insertSharedPin(pin, [ TOPIC ], config.ENV.API_KEY_SANDBOX, log.info); 
  fcm.post('News Headlines', 'news_headlines__latest', `${pin.layout.title} - ${pin.layout.body}`);
}

function download() {
  request.get('http://feeds.bbci.co.uk/news/rss.xml', (err, response, body) => {
    if(cacheFirst) {
      log.info('Caching on first run');
      cacheFirst = false;
      return;
    }

    const stories = getStories(body);
    if(stories.length < 1) return;

    for(var i = 0; i < MAX_PUSHED; i++) pushPin(stories, i);
  });
};

module.exports = { download };
