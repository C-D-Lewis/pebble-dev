/*
 * Use this Spark Core firmware to calibrate your servos for the main program.
 * @Chris_DL
 * 
 * Servos are modified to rotate continuously when given a target, thus the 
 * desired behavior is that 90 dgerees is 'stop'. Achieve this by running this 
 * firmware and tuning the rear screw until the servo stops.
 */

Servo leftServo, rightServo;

void setup() {
    leftServo.attach(A0);
    rightServo.attach(A1);
}

void loop() {
    delay(1000);
    leftServo.write(90);
    rightServo.write(90);
}