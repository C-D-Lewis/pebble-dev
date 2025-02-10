const request = require('request');
const timeline = require('pebble-timeline-js-node');

const config = require('../node-common').config();
const log = require('../node-common').log();
const fcm = require('../node-common').fcm();

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

const STATE_GOOD_SERVICE = 'Good Service';  // TFL string

var cacheFirst = true;  // Don't send pins when the server is launched
var lastStates = [];    // Array of objects describing current line state
var lastPinBody = '';

function pushPin(pin) {
  // Pins only channel
  const TOPIC_PINS = 'delays';
  log.info(`Pushing new pin:\n${JSON.stringify(pin)}\n\n`);
  timeline.insertSharedPin(pin, [ TOPIC_PINS ], config.ENV.API_KEY_PROD, log.info);
  timeline.insertSharedPin(pin, [ TOPIC_PINS ], config.ENV.API_KEY_SANDBOX, log.info);
  fcm.post('Tube Status', 'tube_status__latest', `${pin.layout.title} - ${pin.layout.body}`);

  // Notifs channel
  pin.updateNotification = {
    time: new Date().toISOString(),
    layout: {
      type: 'genericNotification',
      title: 'Tube Delay Update',
      tinyIcon: 'system://images/GENERIC_WARNING',
      body: pin.layout.body,
      foregroundColor: '#FFFFFF',
      backgroundColor: '#0000AA'
    }
  };
  
  const TOPIC_NOTIFS = 'notifs';
  log.info(`Pushing new notif pin:\n${JSON.stringify(pin)}\n\n`);
  timeline.insertSharedPin(pin, [ TOPIC_NOTIFS ], config.ENV.API_KEY_PROD, log.info);
  timeline.insertSharedPin(pin, [ TOPIC_NOTIFS ], config.ENV.API_KEY_SANDBOX, log.info);
}

function buildBody(lines) {
  var body = '';
  for(var i = 0; i < lines.length; i++) {
    if(lines[i].name !== lastStates[i].name) {
      log.error('Cannot compare lines, lists are not in sync.');  // Should never happen
      return;
    }

    if(lines[i].state !== STATE_GOOD_SERVICE) {
      body += `${lines[i].name}: ${lines[i].state}\n`;
      if(lines[i].state !== lastStates[i].state) lastStates[i].state = lines[i].state;
    }
  }
  return body;
}

function processNewDelays(lines) {
  if(lines.length !== lastStates.length) {
    log.error('Cannot compare lines, lengths are unequal.');
    return;
  }

  var body = buildBody(lines);
  var now = new Date();
  if(body.length !== lastPinBody.length) {
    // Something changed
    const PIN_ID = 'tube-status-delays';
    if(!lines.find((line) => line.state !== STATE_GOOD_SERVICE)) {
      body = 'All delays resolved.';
      lastPinBody = '';

      pushPin({
        id: PIN_ID,
        time: now.toISOString(),
        layout: {
          type: 'genericPin',
          tinyIcon: 'system://images/NOTIFICATION_FLAG',
          title: 'No Delays!',
          body: 'Good service on all lines.',
          foregroundColor: '#FFFFFF',
          backgroundColor: '#0000AA'
        }
      });
    } else {
      log.debug(`Delays are: ${body}`);
      lastPinBody = body;

      pushPin({
        id: PIN_ID,
        time: now.toISOString(),
        layout: {
          type: 'genericPin',
          tinyIcon: 'system://images/GENERIC_WARNING',
          title: 'Some Delays',
          body: body,
          foregroundColor: '#FFFFFF',
          backgroundColor: '#0000AA'
        }
      });
    }
  } else log.debug('No change in status.');
}

function download() {
  request('https://api.tfl.gov.uk/line/mode/tube/status', (err, response, body) => {
    log.debug('Download from unified API complete!');

    if(body.includes('DOCTYPE')) {
      log.error(`API may be down, ignoring response:\n${body}`);
      return;
    }

    const json = JSON.parse(body);
    const lines = [];
    for(var i = 0; i < json.length; i++) {
      lines[i] = {
        name: json[i].name, 
        state: json[i].lineStatuses[0].statusSeverityDescription
      };
      lastStates[i] = {
        name: lines[i].name, 
        state: lines[i].state
      };
    
      if(cacheFirst) log.debug(`${lines[i].name}:${lines[i].state}`);
    }

    if(cacheFirst) {
      cacheFirst = false;
      log.debug('Caching initial line states.');
      return;
    }

    processNewDelays(lines);  // Compare to find new delays
  });
}

module.exports = { download };
