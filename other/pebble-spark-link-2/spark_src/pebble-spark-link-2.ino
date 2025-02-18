static bool pressed = false;

void setup() {
    pinMode(D0, INPUT);
}

void loop() {
    //Publish button state
    if(digitalRead(D0) == HIGH && pressed == false)
    {
        Spark.publish("button_state", "HIGH");
        pressed = true;
    }
    else if(digitalRead(D0) == LOW && pressed == true)
    {
        Spark.publish("button_state", "LOW");
        pressed = false;
    }
    
    //Rate limit to prevent spamming the cloud
    delay(500);
}