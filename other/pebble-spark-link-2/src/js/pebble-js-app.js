var accessToken = "";

Pebble.addEventListener("ready",
	function(e) {
		//Register EventSource listener
		var core = new EventSource("https://api.spark.io/v1/events/?access_token=" + accessToken);
		core.addEventListener("button_state", 
			function(response) {
				//Interpret response as JSON
				var json = JSON.parse(response.data);

				console.log("Payload is '" + json.data + "'");

				//Send the payload
				Pebble.sendAppMessage(
					{"KEY_BUTTON_STATE":json.data},
					function(e) {
						console.log("Sent '" + json.data + "' to Pebble.");
					},
					function(e) {
						console.log("Failed to send data to Pebble!");
					}
				);
			}, 
			false
		);

		console.log("Pebble JS Ready!");
	}
);
