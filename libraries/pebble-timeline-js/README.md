# pebble-timeline-js

Simple JS package to allow inserting of personal (user) timeline pins or
AppGlances from PebbleKit JS.

## Installation

`pebble package install pebble-timeline-js`


## Methods

`insertUserPin(pin, callback)` - insert a timeline pin for the user running the app.

`deleteUserPin(pin, callback)` - delete a pin previously pushed. The `id` must match.

`setAppGlances(slices, callback)` - set the current collection of AppGlance slice objects.


## Example Usage

In `src/pkjs/index.js`:

**Push a user pin**

```js
var timelinejs = require('pebble-timeline-js');

var pin = {
  id: 'example-pin-generic-d2sd3d5bdsd',
  time: new Date().toISOString(),
  layout: {
    type: 'genericPin',
    title: 'timelinejs pin test',
    tinyIcon: 'system://images/NOTIFICATION_FLAG'
  }
};


Pebble.addEventListener('ready', function() {
  console.log('Ready! Inserting user pin ' + JSON.stringify(pin));

  timelinejs.insertUserPin(pin, function(responseText) {
    console.log('Result: ' + responseText);
  });
});
```

**Set an AppGlance**

```js
var glances = [
  {
    layout: {
      subtitleTemplateString: 'Hello from AppGlance REST!'
    }
  }
];

timelinejs.setAppGlances(glances, function(responseText) {
  console.log('AppGlance result: ' + responseText);
});
```
