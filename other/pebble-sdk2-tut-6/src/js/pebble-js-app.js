function HTTPGET(url) {
	var req = new XMLHttpRequest();
	req.open("GET", url, false);
	req.send(null);
	return req.responseText;
}

var getWeather = function() {
	//Get weather info
	var response = HTTPGET("http://api.openweathermap.org/data/2.5/weather?q=London,uk");
		
	//Convert to JSON
	var json = JSON.parse(response);
	
	//Extract the data
	var temperature = Math.round(json.main.temp - 273.15);
	var location = json.name;
	
	//Console output to check all is working.
	console.log("It is " + temperature + " degrees in " + location + " today!");
	
	//Construct a key-value dictionary	
	var dict = {"KEY_LOCATION" : location, "KEY_TEMPERATURE": temperature};
	
	//Send data to watch for display
	Pebble.sendAppMessage(dict);
};

Pebble.addEventListener("ready",
  function(e) {
	//App is ready to receive JS messages
	getWeather();
  }
);

Pebble.addEventListener("appmessage",
  function(e) {
	//Watch wants new data!
	getWeather();
  }
);