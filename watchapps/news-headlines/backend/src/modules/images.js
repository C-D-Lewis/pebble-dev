// var imagemagick = require('imagemagick-native');

const log = require('../node-common').log();

// var downloadBinaryResource = function(imgUrl, callback, failedCallback) {
//   request({'uri': imgUrl, 'encoding': null}, function(error, response, body) {
//     if(error) {
//       failedCallback(error);
//       return;
//     }

//     callback(body);
//   });
// };

// var shiftDown = function(val) {
//   return val >>= 14;  // Magic value that works!
// }

function getPixels(png) {
  return [];  // Removed when 144x81 images were removed from the feed

  // From [{r,g,b,a==65535}] to uint8t
  // var info = imagemagick.identify({'srcData': png});
  // log.debug('info: ' + JSON.stringify(info));

  // var arr = imagemagick.getConstPixels({
  //   'srcData': png,
  //   'x': 0,
  //   'y': 0,
  //   'columns': info.width,
  //   'rows': info.height
  // });

  // // Convert palette
  // for(var i = 0; i < arr.length; i += 1) {
  //   var c = arr[i];
  //   c.red = shiftDown(c.red);
  //   c.green = shiftDown(c.green);
  //   c.blue = shiftDown(c.blue);

  //   var b = 0xC0; // 11000000
  //   b = b | (c.red << 4);
  //   b = b | (c.green << 2);
  //   b = b | c.blue;
  //   arr[i] = b; // Replace value
  // }

  // // Convert to array
  // var result = [];
  // var uint_arr = new Uint8Array(arr);
  // for(var i = 0; i < uint_arr.byteLength; i += 1) {
  //   result.push(uint_arr[i]);
  // }

  // return result;
}

function handleImageRequest(req, res) {
  log.debug(`Convert requested: ${req.query.url}`);
  const start = new Date().getTime();

  log.debug('Imagemagick is removed, doing nothing.');
  res.send([0, 0, 0]);

  // request({'uri': req.query.url, 'encoding': null}, function(error, response, body) {
  //   if(!error) {
  //     try {
  //       var png = imagemagick.convert({
  //         'srcData': body,
  //         'srcFormat': 'jpg',
  //         'format': 'png'
  //       });

  //       var pixels = getPixels(png);

  //       res.send(pixels);

  //       var finish = new Date().getTime();
  //       log.debug('Sent after ' + (finish - start) + ' ms.');
  //     } catch(error) {
  //       log.debug('Error converting image: ' + error);
  //       var result = [0, 0, 0];
  //       res.send(result);
  //     }
  //   } else {
  //     log.debug('Error downloading image data: ' + error);
  //   }
  // });
}

module.exports = { handleImageRequest };
