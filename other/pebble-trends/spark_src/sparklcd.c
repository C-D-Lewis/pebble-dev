// This #include statement was automatically added by the Spark IDE.
#include "LiquidCrystal.h"

//Vdd = 5V
//Pin 3 -> 1KOhm -> GND

                //RS,  E, D4, D5, D6, D7
LiquidCrystal lcd(D5, D4, D3, D2, D1, D0);

static int flash_counter = 0;
static int backlight_state = LOW;

void printTruncated(String args)
{
    //If small enough
    if(args.length() < 33)
    {
        //If more than one line needed
        if(args.length() > 16)
        {
            lcd.print(args.substring(0, 16));
            lcd.setCursor(0, 1);
            lcd.print(args.substring(16, 32));
        }
        
        //Can use just top line
        else
        {
            lcd.print(args);
        }
    }
    
    //Show with truncation
    else
    {
        lcd.print(args.substring(0, 16));
        lcd.setCursor(0, 1);
        lcd.print(args.substring(16, 30));
        lcd.print("...");
    }    
}

void printWords(String msg)
{
    if(msg.length() > 16)
    {
        //Check overrun
        if(msg.charAt(15) != ' ' && msg.charAt(16) != ' ')
        {
            //Find start of offending word - TODO: Word whole line ling duplicates beloe
            int index = 15;
            while(msg.charAt(index) != ' ' && index >= 0)
            {
                index--;    
            }
            
            String one = msg.substring(0, index);
            String two = msg.substring(index + 1);
            
            lcd.print(one);
            lcd.setCursor(0, 1);
            lcd.print(two);
        }
        else
        {
            printTruncated(msg);
        }
    }
    else
    {
        printTruncated(msg);
    }
}

int toggle(String args)
{
    if(backlight_state == HIGH)
    {
        digitalWrite(D6, LOW);
        backlight_state = LOW;
    }
    else
    {
        digitalWrite(D6, HIGH);
        backlight_state = HIGH;
    }
}

int set(String args) 
{
    //Clear LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    
    //Replace characters
    args.replace("%20", " ");
    args.replace("%27", "'");
    
    printWords(args);
    
    //Schedule flash
    flash_counter = 5;
    
    return 0;
}

void setup() 
{
    lcd.begin(16, 2);
    
    set("Ready.");
    
    Spark.function("set", set);
    Spark.function("toggle", toggle);
    Spark.variable("state", &backlight_state, INT);
    
    pinMode(D6, OUTPUT);
    digitalWrite(D6, LOW);
}

void loop() 
{
    while(flash_counter > 0)
    {
        digitalWrite(D6, HIGH);
        delay(200);
        digitalWrite(D6, LOW);
        delay(200);
        flash_counter--;
        
        if(flash_counter == 0)
        {
            digitalWrite(D6, backlight_state);
        }
    }
}