Pebble.on('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('Received message: ' + JSON.stringify(e.payload));

  // Reply
  Pebble.sendAppMessage({
    INBOUND: 'Hello from JS!'
  }, function() {
    console.log('Reply sent successfully!');
  }, function() {
    console.log('Failed to send message.');
  });
});
