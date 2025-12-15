// TODO: catch throws from sendAppMessage()
// TODO: Use proper messageKeys

/** News story */
type Story = {
  title?: string;
  description?: string;
};

/** AppMessage data */
type Payload = Record<string, any>;

/** Max feed items the app will display */
const MAX_ITEMS = 20;

/*********************************** Enums ************************************/

const AppKey = {
  Title: 0,               // Story title
  Description: 1,         // Story description
  Quantity: 2,            // Total number of stories
  Index: 3,               // Which story this is
  Ready: 4,               // JS is ready
  SettingsCategory: 5,    // Selected category
  SettingsNumStories: 6,  // Number of stories to show
  SettingsRegion: 7       // Selected region
};

const Region = {
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

/******************************** Pebble helpers ******************************/

const hasKey = (e: AppMessageEvent, key: number) => {
  return typeof e.payload[key] !== 'undefined';
};

const getValue = (e: AppMessageEvent, key: number): string => {
  if (hasKey(e, key)) return String(e.payload[key]);
  
  throw new Error('getValue(): Key ' + key + ' does not exist in received dictionary!');
};

const getInt = (e: AppMessageEvent, key: number): number => {
  return parseInt(getValue(e, key));
};

/******************************** BBC News API ********************************/

let gStories: Story[] = [];
let gLastIndex = 0;
let gQuantity = 0;
let gCategory = 0;
let gRegion = 0;
let gLastOffset = 0;

const decode = (str: string) => {
  str = str.replace(/&amp;/g, '&');
  str = str.replace('<![CDATA[', '');
  str = str.replace(']]>', '');
  return str;
};

const parseFeed = (responseText: string) => {
  const items = [];
  let outerSpool = responseText;

  // Strip heading data
  outerSpool = outerSpool.substring(outerSpool.indexOf('<item>'));

  while(outerSpool.indexOf('<title>') > 0 && items.length < gQuantity) {
    const s: Story = {};
    const spool = outerSpool.substring(0, outerSpool.indexOf('</item>'));

    // Title
    let title = spool.substring(spool.indexOf('<title>') + '<title>'.length);
    title = title.substring(0, title.indexOf('</title>'));
    if (title.indexOf('VIDEO') > -1) {
      title = title.substring(7);
    }
    s.title = decode(title);

    // Desc
    let desc = spool.substring(spool.indexOf('<description>') + '<description>'.length);
    desc = desc.substring(0, desc.indexOf('</description>'));
    s.description = decode(desc);

    // Add
    items.push(s);
    // console.log(JSON.stringify(s));

    // Next
    outerSpool = outerSpool.substring(outerSpool.indexOf('</item>') + '</item>'.length);
  }

  console.log(`parseFeed(): Extracted ${items.length} items.`);
  return items;
};

const getUKRegionCategoryURL = (input: number): string => {
  let category;
  switch(input) {
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
      console.log('Defaulting to headlines for category: ' + input);
      category = 'headlines';
      break;
  }

  // Choose URL based on category choice
  if (category == 'headlines') return 'http://feeds.bbci.co.uk/news/rss.xml';
  return `http://feeds.bbci.co.uk/news/${category}/rss.xml`;
};

const getURL = () => {
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
      console.log(`getURL(): Region is invalid or is UK: ${gRegion}, defaulting to selected category ${gCategory}`);
      return getUKRegionCategoryURL(gCategory);
  }
};

/********************************** App Transfer ******************************/

// Upload one story at a time
const sendNextStory = async () => {
  if (gLastIndex === gQuantity) {
    console.log('sendNextStory(): Sent all stories to Pebble!');
    return;
  }

  const dict: Payload = {};
  dict[AppKey.Index] = gLastIndex;
  dict[AppKey.Title] = gStories[gLastIndex].title;
  dict[AppKey.Description] = gStories[gLastIndex].description;

  await PebbleTS.sendAppMessage(dict);
  gLastIndex += 1;
  sendNextStory();
};

const sendToWatch = async (responseText: string) => {
  // User pref else dict length if less than max
  console.log(`sendToWatch(): initial quantity: ${gQuantity}`);

  // Strip metadata
  responseText = responseText.substring(responseText.indexOf('<item>') + '<item>'.length);
  gStories = parseFeed(responseText);

  // There are more than MAX
  if (gQuantity > MAX_ITEMS) {
    console.log(`sendToWatch(): quantity > MAX_ITEMS, now ${MAX_ITEMS}`);
    gQuantity = MAX_ITEMS;
  }

  // There are not enough
  if (gQuantity > gStories.length) {
    console.log(`sendToWatch(): gQuantity > gStories.length, now ${gStories.length}`);
    gQuantity = gStories.length;
  }

  // Start upload
  const dict: Payload = {};
  dict[AppKey.Quantity] = gQuantity;

  await PebbleTS.sendAppMessage(dict);
  console.log(`sendToWatch(): Quantity ${gQuantity} sent, beginning upload.`);
  gLastIndex = 0;
  await sendNextStory();
};

/********************************** PebbleKit JS ******************************/

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready!');

  const dict: Payload = {};
  dict[AppKey.Ready] = 1;

  await PebbleTS.sendAppMessage(dict);
  console.log('Sent ready to watchapp.');
});

Pebble.addEventListener('appmessage', async (e) => {
  console.log(`appmessage: ${JSON.stringify(e.payload)}`);

  // Settings - if category exists, the others will too
  if (hasKey(e, AppKey.SettingsCategory)) {
    gCategory = getInt(e, AppKey.SettingsCategory);
    gQuantity = getInt(e, AppKey.SettingsNumStories);
    gRegion = getInt(e, AppKey.SettingsRegion);
    console.log(`appmessage: Watch sent settings: ${gCategory}/${gQuantity}/${gRegion}`);
    
    const text = await fetch(getURL()).then(r => r.text());
    await sendToWatch(text);
  }
});
