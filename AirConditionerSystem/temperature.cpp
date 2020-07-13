float temp_waterIn;
float temp_waterOut;
float temp_freonIn;
float temp_freonOut;
float temp_digital;
float temp_inside;
float temp_heatExchange;

#define waterInPin A0
#define waterOutPin A1
#define freonInPin A2
#define freonOutPin A3
#define heatExchangeTempPin A7

// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000
//time between each temperature check
#define tempCheckDelayTime 1000

#include <OneWire.h>
#include <DallasTemperature.h>
/********************************************************************/
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 9
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


int pin = 5;
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(pin, OUTPUT);
  Serial.begin(9600);

  pinMode(waterInPin, INPUT);
    pinMode(waterOutPin, INPUT);
    pinMode(freonInPin, INPUT);
    pinMode(freonOutPin, INPUT);
    pinMode(heatExchangeTempPin, INPUT);
    sensors.begin();
}

// the loop function runs over and over again forever
void loop() {
  checkTemp();

  Serial.print(temp_waterIn);
Serial.print(" | ");
Serial.print(temp_waterOut);
Serial.print(" | ");
Serial.print(temp_freonIn);
Serial.print(" | ");
Serial.print(temp_freonOut);
Serial.print(" | ");
Serial.print(temp_heatExchange);
Serial.print(" | ");
Serial.print(temp_inside);
Serial.print(" | ");
Serial.print(temp_digital);
Serial.println(" | ");

delay(1000);

}

void checkTemp()
{
    if(1)
    {
        temp_waterIn = round(getTemperature(waterInPin) * 10.0) / 10.0;
        temp_waterOut = round(getTemperature(waterOutPin) * 10.0) / 10.0;
        temp_freonIn = round(getTemperature(freonInPin) * 10.0) / 10.0;
        temp_freonOut = round(getTemperature(freonOutPin) * 10.0) / 10.0;
        temp_heatExchange = round(getTemperature(heatExchangeTempPin) * 10.0) / 10.0;
        sensors.requestTemperatures();
        temp_inside = sensors.getTempCByIndex(0);
        temp_digital = sensors.getTempCByIndex(1);
    }

}

float getTemperature(int pin)
{
    float reading = 0;

    for(int i = 0; i < NUMSAMPLES; i++)
    {
        reading += analogRead(pin);
        delay(2); //this delay is potential problem
    }

    reading = reading / float(NUMSAMPLES);

    // convert the value to resistance
    reading = (1023 / reading)  - 1;     // (1023/ADC - 1)
    reading = SERIESRESISTOR * reading;  // 10K / (1023/ADC - 1)

    float steinhart;
    steinhart = reading / THERMISTORNOMINAL;     // (R/Ro)
    steinhart = log(steinhart);                  // ln(R/Ro)
    steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                 // Invert
    steinhart -= 273.15;                         // convert to C
    return steinhart;
}
