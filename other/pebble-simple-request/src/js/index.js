// Internal
function request(url, type, data, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);

  if(data === null) {
    xhr.setRequestHeader('Content-Type', 'text/plain');
    xhr.send('');
  } else {
    var type = (typeof data).toLowerCase();
    if(type === 'object') {
      xhr.setRequestHeader('Content-Type', 'application/json');
      xhr.send(JSON.stringify(data));
    } else if(type === 'string') {
      xhr.setRequestHeader('Content-Type', 'text/plain');
      xhr.send(data);
    }
  }
}

/**
 * Send a GET request.
 * Parameters:
 *   url  - URL of the website to send a GET request to.
 *   body - Any data necessary in the request body as object or string. Use null if not needed.
 *   callback - A function that will get the XHR.responseText as a single parameter.
 **/
function get(url, body, callback) {
  request(url, 'GET', body, callback);
}

/**
 * Send a POST request.
 * Parameters:
 *   url  - URL of the website to send a POST request to.
 *   body - Any data necessary in the request body as object or string. Use null if not needed.
 *   callback - A function that will get the XHR.responseText as a single parameter.
 **/
function post(url, body, callback) {
  request(url, 'POST', body, callback);
}

exports.get = get;
exports.post = post;
