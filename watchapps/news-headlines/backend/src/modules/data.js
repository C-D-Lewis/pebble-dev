/* eslint-disable no-param-reassign */
const fetch = require('node-fetch');
// const timelinejs = require('pebble-timeline-js-node');

const { API_KEY_PROD } = process.env;

/** API URL for RWS timeline */
const API_URL_ROOT = 'https://timeline-api.rebble.io/';
/** Max pins pushed each INTERVAL. Prevents infamous timeline blob db errors. */
const MAX_PUSHED = 1;
/** Max 'already pushed' stories */
const MAX_DUPLICATES = 50;

// Check new stories against the last MAX_DUPLICATES to prevent hour-later duplucates
const dupeBuffer = [];
// Don't post pins right away - stateful
let cached = false;

/**
 * Extract text from a larger block with locator strings.
 *
 * @param {string} text - Text to search through.
 * @param {Array<string>} befores - Sequential samples to find before the text to extract.
 * @param {string} after - Sample immediately after text to extract.
 * @returns {string} Extracted text.
 */
const extract = (text, befores, after) => {
  let copy = `${text}`;

  befores.forEach((item) => {
    const start = copy.indexOf(item);
    if (start === -1) throw new Error(`Unable to find ${item} when extracting`);

    copy = copy.substring(start);
  });

  copy = copy.substring(befores[befores.length - 1].length);
  return copy.substring(0, copy.indexOf(after));
};

/**
 * Decode XML entities.
 *
 * @param {string} str - String to decode.
 * @returns {string} Decoded string.
 */
const decode = (str) => str
  .split(/&amp;/g)
  .join('&')
  .split('<![CDATA[')
  .join('')
  .split(']]>')
  .join('');

/**
 * Get stories from an XML string.
 *
 * @param {string} xml - XML string to extract stories from.
 * @returns {Array} Extracted stories.
 */
const getStories = (xml) => {
  const items = [];
  xml = xml.split('<item>');
  xml.shift();
  xml.forEach((xmlChunk) => {
    const story = {
      title: decode(extract(xmlChunk, ['<title>'], '</title>')),
      description: decode(
        extract(xmlChunk, ['<description>'], '</description>'),
      ),
      date: extract(xmlChunk, ['<pubDate>'], '</pubDate>'),
    };

    if (!dupeBuffer.find((dupe) => dupe.title === story.title)) {
      dupeBuffer.unshift(story);
      if (dupeBuffer.length > MAX_DUPLICATES) dupeBuffer.pop();

      items.push(story);
      console.log(`Added new story: \n${story.title}\n${story.description}\n`);
    }
  });

  console.log(`There are ${dupeBuffer.length} items in the duplicate buffer`);
  console.log(`Extracted ${items.length} items.`);
  return items;
};

/**
 * Insert a pin into the timeline, user (non-shared mode)
 *
 * @param {object} pin - Pin to insert.
 */
const insertUserPin = async (pin) => {
  if (!API_KEY_PROD) throw new Error('API_KEY_PROD is not set in .env');

  const url = `${API_URL_ROOT}v1/user/pins/${pin.id}`;

  const res = await fetch(url, {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json',
      'X-User-Token': API_KEY_PROD,
    },
    body: JSON.stringify(pin),
  });

  const body = await res.text();
  if (body !== 'OK') {
    console.log(`Failed to insert pin: ${body}`);
    return;
  }
  console.log(`Inserted pin: ${pin.id}`);
};

/**
 * Push a pin to the timeline.
 *
 * @param {Array} stories - Array of stories to push.
 * @param {number} index - Index of the story to push.
 */
const pushPin = (stories, index) => {
  const pubDate = new Date(stories[index].date);
  const pin = {
    id: `news-headlines-story-${pubDate.getTime()}`,
    time: pubDate.toISOString(),
    layout: {
      type: 'genericPin',
      tinyIcon: 'system://images/NEWS_EVENT',
      title: stories[index].title,
      subtitle: 'BBC News',
      body: stories[index].description,
      foregroundColor: '#FFFFFF',
      backgroundColor: '#AA0000',
    },
  };
  console.log(`pin=${JSON.stringify(pin, null, 2)}`);

  // Add pin for this user
  insertUserPin(pin);

  // FIXME: Rebble timeline API doesn't support shared topics yet
  // const TOPIC = 'headlines';
  // timelinejs.insertSharedPin(pin, [TOPIC], API_KEY_PROD, console.log);
  // timelinejs.insertSharedPin(pin, [TOPIC], API_KEY_SANDBOX, console.log);
};

/**
 * Download the BBC News RSS feed and push pins to the timeline.
 */
const download = async () => {
  const res = await fetch('http://feeds.bbci.co.uk/news/rss.xml');
  const body = await res.text();

  if (!cached) {
    console.log('Caching on first run');
    cached = true;
    return;
  }

  const stories = getStories(body);
  if (stories.length < 1) return;

  for (let i = 0; i < MAX_PUSHED; i += 1) pushPin(stories, i);
};

module.exports = { download };
