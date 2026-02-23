# pebble-simple-request

Simple get and post request library for Pebble apps. Mostly for me to do simple
`GET` and `POST` with JSON bodies, but others may find it useful.


## How to use

1. `pebble package install pebble-simple-request`

2. In your JS file:
    ```
    var request = require('pebble-simple-request');
    ```

3. `GET` with no body data (2nd param):
    ```
    request.get('https://example.com', null, function(responseText) {
      console.log(responseText);
    });
    ```

4. `POST` with a JSON body:
    ```
    var obj = { example: 0 };
    request.post('http://examplepostservice.com/post', obj, function(responseText) {
      console.log(responseText);
    });
    ```

## Changelog

**1.0.0**
- Inital version

**1.1.0**
- Build for Emery
