var DEVICE_ID = '';
var ACCESS_TOKEN = '';

var importjQuery = function() {
  var script = document.createElement('script');
  script.src = 'http://code.jquery.com/jquery-latest.min.js';
  script.type = 'text/javascript';
  document.getElementsByTagName('head')[0].appendChild(script);
};

Pebble.addEventListener('ready',
  function(e) {
    importjQuery();
    console.log('Pebble JS Ready!');
  }
);

Pebble.addEventListener('appmessage',
  function(dict) {
    console.log('AppMessage received!');

    //If AppMessage contains KEY_TOGGLE
    if(typeof dict.payload['KEY_TOGGLE'] !== 'undefined') {
      console.log('KEY_TOGGLE received!');

      //Construct POST URL
      var url = 'https://api.spark.io/v1/devices/' + DEVICE_ID + '/toggle?access_token=' + ACCESS_TOKEN;

      //Send with jQuery
      $.ajax({
        type:'POST',
        url:url,
        data:{'args':'none'},  //No args for the moment
        success:function() {
          console.log('POST successful!');
        },
        dataType:'json'
      });
    }
  }
);
