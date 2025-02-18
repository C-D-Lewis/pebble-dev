pebble-spark-base
=================

Base project to run code on Spark when Pebble buttons are pressed.

This version includes Servo convenience functions for operating the Pebble Dev Retreat robots.

## Instructions

1. Upload `spark_src/main.ino` to your Spark Core.
2. Enter your Core's device ID and access token (found in Spark IDE) in `src/js/pebble-js-app.js`.
3. Compile and install the Pebble app to your watch.
4. Press buttons to run the `up()`, `select()` etc. functions on Spark!

## Setup

The servo connections should be as following (with wires protruding from rear of the robot):

* Red:   Battery +ve

* Black: Battery -ve **and** Spark Core GND.

* White: A0 for left servo, A1 for right servo.

*Note*: The servos will require adjustment to function correctly without feedback.

Use `spark_src/calibrate_servo.ino` for this purpose.
