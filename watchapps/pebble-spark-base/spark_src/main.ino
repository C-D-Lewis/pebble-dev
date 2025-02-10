/*
 * Pebble-Spark Base firmware for Spark Core robots
 * @Chris_DL
 *
 * Receives the button press events from src/pebble-spark-base.c and executes
 * an action in each of up(), select() etc.
 */
 
Servo leftServo, rightServo;

/******************************* Servo helpers ********************************
 * Connections:
 * Both red - Battery pack +ve
 * Both black - Battery pack -ve AND Spark Core GND
 * Left servo white - A0
 * Right servo white - A1
 */

void stop() {
    leftServo.write(90);
    rightServo.write(90);
}

void backwards() {
    leftServo.write(0);
    rightServo.write(180);
}

void forwards() {
    leftServo.write(180);
    rightServo.write(0);
}

void left() {
    leftServo.write(0);
    rightServo.write(0);
}

void right() {
    leftServo.write(180);
    rightServo.write(180);
}

/****************************** Pebble actions ********************************
 * These will be called by src/js/pebble-js-app.js when each button is pressed.
 * 
 * Enter the code you want to run when a button is pressed!
 */

int select(String args) {
    forwards();
    return 0;
}

int down(String args) {
    right();
    return 0;
}

int longselect(String args) {
    stop();
    return 0;
}

int up(String args) {
    left();
    return 0;
}

/****************************** Spark main ************************************/

// Runs once at the start
void setup() {
    // Initialize servos
    leftServo.attach(A0);
    rightServo.attach(A1);
    
    // Expose functions to the Spark Cloud to receive Pebble events
    Spark.function("select", select);
    Spark.function("down", down);
    Spark.function("longselect", longselect);
    Spark.function("up", up);
}

// Loops forever until reset or power-off
void loop() {

}