var deviceId = "";
var accessToken = "";

/**************** Pebble helpers ****************/

var hasKey = function(dict, key) {
	return typeof dict.payload[key] !== "undefined";
};

var getValue = function(dict, key) {
	if(hasKey(dict, key)) {
		return "" + dict.payload[key];
	} else {
		console.log("ERROR: Key '" + key + "' does not exist in received dictionary");
		return undefined;
	}
};

/****************** XHR *************************/

//Spark.publish("channel", agrs) reports that args in int(String args) is 'null' with this method (why?)
var xhrRequest = function (url, type, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload =  function () {
      callback(this.responseText);
	};
	xhr.open(type, url);
	xhr.send();
};

var importjQuery = function() {
	var script = document.createElement('script');
	script.src = 'http://code.jquery.com/jquery-latest.min.js';
	script.type = 'text/javascript';
	document.getElementsByTagName('head')[0].appendChild(script);
};

/****************** Main ************************/

var success = function(json) {
	console.log("Response JSON: " + JSON.stringify(json));

	Pebble.sendAppMessage(
		{"PIN_EVENT":json.return_value},
		function(e) {
			console.log("Pin " + json.return_value + " updated on Pebble");
		},
		function(e) {
			console.log("FAILED to update pin on Pebble!");
		}
	);
};

Pebble.addEventListener("ready",
	function(e) {
		importjQuery();
		console.log("Pebble JS ready!");
	}
);

Pebble.addEventListener("appmessage",
	function(dict) {
		var url = "";
		var pin = "";
		var valid = false;
		
		//On request?
		if(hasKey(dict, "PIN_ON")) {
			console.log("Request to turn pin " + getValue(dict, "PIN_ON") + " ON received.");

			pin = getValue(dict, "PIN_ON");
			
			//Construct URL
			url = "https://api.spark.io/v1/devices/" + deviceId + "/on?access_token=" + accessToken;
			
			valid = true;
		}
		
		//Off request?
		else if(hasKey(dict, "PIN_OFF")) {
			console.log("Request to turn pin " + getValue(dict, "PIN_OFF") + " OFF received.");

			pin = getValue(dict, "PIN_OFF");
			
			//Construct URL
			url = "https://api.spark.io/v1/devices/" + deviceId + "/off?access_token=" + accessToken;

			valid = true;
		}
		
		//Invalid
		else {
			console.log("Invalid key received!");
		}
			
		if(valid === true)
		{
			//Send with XHR
			// xhrRequest(url, "POST", 
			// 	function(responseText) {
			// 		console.log("Response text: " + responseText);
					
			// 		var json = JSON.parse(responseText);
	
			// 		Pebble.sendAppMessage(
			// 			{"PIN_EVENT":json.return_value},
			// 			function(e) {
			// 				console.log("Pin " + json.return_value + " updated on Pebble");
			// 			},
			// 			function(e) {
			// 				console.log("FAILED to update pin on Pebble!");
			// 			}
			// 		);
			// 	}
			// );

			console.log("ajax");

			//Send with jQuery
			$.ajax({
			  type: "POST",
			  url: url,
			  data: {"args":pin},
			  success: success,
			  dataType: "json"
			});
		}
	}
);
