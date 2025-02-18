/*
 * Pebble.js-Spark Base firmware
 * @Chris_DL
 *
 * Receives the button press events from src/app.js and executes
 * an action in each of up(), select() etc.
 */

 static bool state;

 void toggle() {
  if(state) {
    digitalWrite(D7, false);
    state = false;
  } else {
    digitalWrite(D7, true);
    state = true;
  }
 }
 
/****************************** Pebble actions ********************************
 * These will be called by src/app.js when each button is pressed.
 * 
 * Enter the code you want to run when a button is pressed!
 */

int select(String args) {
  toggle();
  return 0;
}

int down(String args) {
  toggle();
  return 0;
}

int up(String args) {
  toggle();
  return 0;
}

/****************************** Spark main ************************************/

// Runs once at the start
void setup() {
  // Show actions on on-board LED
  pinMode(D7, OUTPUT);

  // Expose functions to the Spark Cloud to receive Pebble events
  Spark.function("select", select);
  Spark.function("down", down);
  Spark.function("up", up);
}

// Loops forever until reset or power-off
void loop() {

}