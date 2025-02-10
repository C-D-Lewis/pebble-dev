bool is_high = false;

int toggle(String args)
{
    if(is_high == true)
    {
        digitalWrite(D7, LOW);
        is_high = false;
    }
    else
    {
        digitalWrite(D7, HIGH);
        is_high = true;
    }
    
    return 0;
}

void setup() {
    pinMode(D7, OUTPUT);
    Spark.function("toggle", toggle);
}

void loop() {
    //Nothing here
}