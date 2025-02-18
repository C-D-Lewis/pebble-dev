// Spark Function must be int(String s);
int on(String args)
{
    int pin = args.toInt();

    digitalWrite(pin, HIGH);

    return pin;
}

int off(String args)
{
    int pin = args.toInt();

    digitalWrite(pin, LOW);

    return pin;
}

void setup()
{
    //Setup pin modes for D0 to D7
    for(int i = 0; i < 8; i++)
    {
        pinMode(i, OUTPUT);
    }

    //Export functions to cloud
    Spark.function("on", on);
    Spark.function("off", off);
}

void loop()
{
    //Nothing here
}