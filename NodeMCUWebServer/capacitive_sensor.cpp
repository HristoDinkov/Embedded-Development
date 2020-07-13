#include <CapacitiveSensor.h>

CapacitiveSensor   cs_4_2 = CapacitiveSensor(4,2);        // 10 megohm resistor between pins 4 & 2, pin 2 is sensor pin, add wire, foil

bool lamp = false;
bool isPressed = false;

void setup()
{
   cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
   Serial.begin(9600);

   pinMode(13, OUTPUT);
}

void loop()
{

    long total1 =  cs_4_2.capacitiveSensor(30);
    Serial.println(total1);                  // print sensor output 1
    if(total1 > 400 && isPressed == false)
    {
        lamp = !lamp;
        digitalWrite(13, lamp);
        isPressed = true;
    }else if(total1 < 200 && isPressed)
    {
      isPressed = false;
    }

    delay(10);                             // arbitrary delay to limit data to serial port
}
