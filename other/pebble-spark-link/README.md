# pebble-spark-link

Sample project to invoke a Spark.function() exposed function from a Pebble watch.

## How To Use

1. Set `DEVICE_ID` and `ACCESS_TOKEN` in `js/pebble-js-app.js` according to those for your Spark Core and account.
2. Compile and run the Pebble watchapp on your Pebble.
3. Upload and flash `spark_src/pebble-spark-link.ino` to your Spark Core
4. Press button, toggle LED on D7!

![screenshot](screenshot/screen1.png)