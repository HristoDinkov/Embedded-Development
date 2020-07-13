/*
    screens:
        0: Problem Screen
        1: Temperature Screen
        2: Flow Rate Screen

    problems:
        1: Low Heat Exchange
        2: Low Flow Rate
*/

#define MIN_TEMP_HEAT_EXCHANGE 5
#define MIN_FLOW_RATE 200
#define CRITICAL_FLOW_RATE 50

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

#include <SoftwareSerial.h> // Arduino IDE <1.6.6
#include <PZEM004T.h>//Watt Meter


PZEM004T pzem(10,11);  // (RX,TX) connect to TX,RX of PZEM
IPAddress ip(192,168,1,1);

#define button 4
#define numberOfScreens 2

#define ledPin 6

#define relay 3
#define pumpRelay 5

#define hallSensorInterrupt 0  // 0 = digital pin 2
#define hallSensorPin 2


// which analog pin to connect
#define waterInPin A0
#define waterOutPin A1
#define freonInPin A2
#define freonOutPin A3
#define digitalTemperaturesPin 9
#define heatExchangeTempPin A6
#define outsideTempPin A7
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


//OneWire oneWire(digitalTemperaturesPin);
//DallasTemperature sensors(&oneWire);



float calibrationFactor = 6.0;
volatile byte pulseCount;
float flowRate;
float flowRate_old;
unsigned long flowRateCheckStartTime;

float temp_waterIn;
float temp_waterOut;
float temp_freonIn;
float temp_freonOut;
float temp_outside;
float temp_heatExchange;
float temp_waterIn_old;
float temp_waterOut_old;
float temp_freonIn_old;
float temp_freonOut_old;
float temp_outside_old;
float temp_heatExchange_old;
long tempCheckStartTime;

bool isButtonPressed = false;
bool wasButtonHold = false;
//bool backlight = false;
long startTime;
//long lightDelayTime = 30000;
long screenDelayTime = 30000;
int screen = 1;
int screen_old = screen;

bool isProblemSeen = false;
bool isProblem = false;
int problem = 0;
int problem_old = problem;

float power = 0;
float energy = 0;
float power_old = 0;
float energy_old = 0;
float current = 0;
float voltage = 0;

int ledDelayTime = 1000;
long ledStartTime = 0;
bool ledStatus = false;

bool pumpStatus = false;

bool isScreenChanged = false;

void setup()
{

    // Initialize a serial connection for reporting values to the host
    Serial.begin(9600);

    pzem.setAddress(ip);

    lcd.begin(20,4);
    lcd.backlight();

    pinMode(button, INPUT);

    pinMode(waterInPin, INPUT);
    pinMode(waterOutPin, INPUT);
    pinMode(freonInPin, INPUT);
    pinMode(freonOutPin, INPUT);
    pinMode(outsideTempPin, INPUT);
    pinMode(heatExchangeTempPin, INPUT);

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);


    pinMode(relay, OUTPUT);
    pinMode(pumpRelay, OUTPUT);
    digitalWrite(relay, HIGH);
    digitalWrite(pumpRelay, LOW);

    pinMode(hallSensorPin, INPUT);
    digitalWrite(hallSensorPin, HIGH);

    pulseCount = 0;
    flowRate = 0.0;
    flowRateCheckStartTime = 0;

    // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
    // Configured to trigger on a FALLING state change (transition from HIGH
    // state to LOW state)
    attachInterrupt(hallSensorInterrupt, pulseCounter, FALLING);

//    sensors.begin();

    checkTemp();
    tempCheckStartTime = millis();

}

/**
 * Main program loop //////////////////////////////////////////////////////////////
 */
void loop()
{
    long begginingTime = millis();

    checkWattMeter();
    checkFlowRate();
    readButton();
    //checkBacklight();
    checkTemp();
    checkPump();
    checkSystem();
    checkLed();
    checkProblemScreen();
    checkScreen();

    if(isScreenChanged){
        lcd.clear();
        isScreenChanged = false;
    }
    switch(screen)
    {
    case 1:
        printTempScreen();
        break;
    case 2:
        printFlowScreen();
        break;
    case 0:
        printProblemScreen();

    }

    if(millis()-begginingTime > 100000)
    {
        resetTimes();
    }
}
/**
 * Main LOOP END ///////////////////////////////////////////////////////////////////
 */

 void checkLed()
 {
     if(isProblem && ledStartTime + ledDelayTime <= millis())
     {
         ledStartTime = millis();
         if(ledStatus){
            digitalWrite(ledPin, LOW);
            ledStatus = false;
         }else{
            digitalWrite(ledPin, HIGH);
            ledStatus = true;
         }
     }

    if(!isProblem)
    {
        digitalWrite(ledPin, LOW);
        ledStatus = false;
    }
 }

 void checkPump()
 {
    if(power > 20 && pumpStatus == false)
    {
        digitalWrite(pumpRelay, HIGH);
        pumpStatus = true;
    }
    else if(pumpStatus == true)
    {
        digitalWrite(pumpRelay, LOW);
        pumpStatus = false;
    }
 }

void checkSystem()
{


    if(temp_heatExchange < MIN_TEMP_HEAT_EXCHANGE)
    {
        digitalWrite(relay, LOW);
        isProblem = true;
        problem = 1;
        if(!isProblemSeen)
        {
            screen = 0;

        }
    }
    else if(flowRate < MIN_FLOW_RATE && pumpStatus == true)
    {
        isProblem = true;
        problem = 2;
        if(!isProblemSeen)
        {
            screen = 0;

        }

        if(flowRate <= CRITICAL_FLOW_RATE)
        {
            digitalWrite(relay, LOW);
        }
    }
    else
    {
        isProblem = false;
        if(screen == 0){
          screen = 1;

        }

    }

    if(temp_heatExchange >= MIN_TEMP_HEAT_EXCHANGE)
    {
        digitalWrite(relay, HIGH);
    }

    if(flowRate > CRITICAL_FLOW_RATE)
    {
        digitalWrite(relay, HIGH);
    }
}

void checkWattMeter(){
    float p = pzem.power(ip);
    if(p >= 0.0)
    {
        power = p;
    }

    float e = pzem.energy(ip);
    if(e >= 0.0)
    {
        energy = e;
    }
}

void checkScreen()
{
    if(temp_heatExchange != temp_heatExchange_old && (screen == 0 || screen == 1) ||
            temp_freonIn != temp_freonIn_old && screen == 1 ||
            temp_freonOut != temp_freonOut_old && screen == 1 ||
            temp_outside != temp_outside_old && screen == 1 ||
            temp_waterIn != temp_waterIn_old && screen == 1 ||
            temp_waterOut != temp_waterOut_old && screen == 1 ||
            flowRate != flowRate_old && (screen == 2 || screen == 0) ||
            power != power_old && screen == 2 ||
            energy != energy_old && screen == 2)
    {
        isScreenChanged = true;

        temp_heatExchange_old = temp_heatExchange;
        temp_freonIn_old = temp_freonIn;
        temp_freonOut_old = temp_freonOut;
        temp_outside_old = temp_outside;
        temp_waterIn_old = temp_waterIn;
        temp_waterOut_old = temp_waterOut;
        flowRate_old = flowRate;
        power_old = power;
        energy_old = energy;
    }

    if(screen != screen_old || problem != problem_old){
      isScreenChanged = true;
      screen_old = screen;
      problem_old = problem;
    }
}

void printProblemScreen()
{
    switch(problem)
    {
    case 1:
        printTempProblemScreen();
        break;
    case 2:
        printFlowProblemScreen();
        break;
    }
}

void printFlowProblemScreen()
{
    lcd.setCursor(0,0);
    lcd.print("Flow rate is low!");
    lcd.setCursor(0,1);
    lcd.print("Flowrate: ");
    lcd.print(flowRate, 0);
}

void printTempProblemScreen()
{
    lcd.setCursor(0,0);
    lcd.print("Exchange temperature");
    lcd.setCursor(0,1);
    lcd.print("is low! Check the");
    lcd.setCursor(0,2);
    lcd.print("system! ");
    lcd.setCursor(0,3);
    lcd.print("Exch.Temp: ");
    lcd.print(temp_heatExchange, 0);
}

void printTempScreen()
{

    lcd.setCursor(0,0);
    lcd.print("Tf.in  Tw.out  Texc");
    lcd.setCursor(0,1);
    lcd.print(temp_freonIn, 1);
    lcd.setCursor(7,1);
    lcd.print(temp_waterOut, 1);
    lcd.setCursor(15,1);
    lcd.print(temp_heatExchange, 1);
    lcd.setCursor(0,2);
    lcd.print(temp_freonOut, 1);
    lcd.setCursor(7,2);
    lcd.print(temp_waterIn, 1);
    lcd.setCursor(15,2);
    lcd.print(temp_outside, 1);
    lcd.setCursor(0,3);
    lcd.print("Tf.out Tw.in   Tout");
}

void printFlowScreen()
{
    float kwIn = power/1000.0;
    float kwh = energy/1000.0;
    float kwOut = flowRate*(temp_waterOut - temp_waterIn)/1000.0;
    float COP = kwOut/kwIn;
    lcd.setCursor(0,0);
    lcd.print("Power   Power  Wdebt");
    lcd.setCursor(0,1);
    lcd.print(" In      Out    m3h ");

    lcd.setCursor(0,2);
    lcd.print(kwIn, 2);
    lcd.print("kW");
    lcd.setCursor(8,2);
    lcd.print(kwOut, 2);
    lcd.print("kW");
    lcd.setCursor(16,2);
    lcd.print(flowRate, 0);

    lcd.setCursor(0,3);
    lcd.print(kwh, 2);
    lcd.print("kWh");
    lcd.setCursor(12,3);
    lcd.print("COP=");
    lcd.print(COP, 2);
}

void readButton()
{
    isButtonPressed = digitalRead(button);


    if(isButtonPressed)
    {
        startTime = millis();
        wasButtonHold = true;
    }
    else if(wasButtonHold)
    {
        wasButtonHold = false;

        if(isProblem){
            isProblemSeen = true;
        }

        //if(backlight == false)
        //{
        //    lcd.backlight();
        //    backlight = true;
        //}
        //else
        //{
        if(screen >= numberOfScreens)
        {
            screen = 1;
        }
        else
        {
            screen++;
        }

        //}

    }
}

//void checkBacklight()
//{
//    if(backlight == true)
//   {
//        if(startTime + lightDelayTime <= millis())
//       {
//            lcd.noBacklight();
//            backlight = false;
//        }
//    }
//}

void checkProblemScreen()
{
    if(startTime + screenDelayTime <= millis() && isProblem)
    {
        screen = 0;
        isProblemSeen = false;
    }
}


/*
Insterrupt Service Routine
 */
void pulseCounter()
{
    // Increment the pulse counter
    pulseCount++;
}

void checkFlowRate()
{
    if((millis() - flowRateCheckStartTime) > 1000)    // Only process counters once per second
    {
        // Disable the interrupt while calculating flow rate and sending the value to
        // the host
        detachInterrupt(hallSensorInterrupt);

        // Because this loop may not complete in exactly 1 second intervals we calculate
        // the number of milliseconds that have passed since the last execution and use
        // that to scale the output. We also apply the calibrationFactor to scale the output
        // based on the number of pulses per second per units of measure (litres/minute in
        // this case) coming from the sensor.
        flowRate = ((1000.0 / (millis() - flowRateCheckStartTime)) * pulseCount) / calibrationFactor;

        // Note the time this processing pass was executed. Note that because we've
        // disabled interrupts the millis() function won't actually be incrementing right
        // at this point, but it will still return the value it was set to just before
        // interrupts went away.
        flowRateCheckStartTime = millis();

        flowRate = flowRate*60;

        // Print the flow rate for this second in litres / hour
        //Serial.println("Flow rate: " + String(flowRate));

        // Reset the pulse counter so we can start incrementing again
        pulseCount = 0;

        // Enable the interrupt again now that we've finished sending output
        attachInterrupt(hallSensorInterrupt, pulseCounter, FALLING);
    }
}


void checkTemp()
{
    if(tempCheckStartTime + tempCheckDelayTime < millis())
    {
        temp_waterIn = round(getTemperature(waterInPin) * 10.0) / 10.0;
        temp_waterOut = round(getTemperature(waterOutPin) * 10.0) / 10.0;
        temp_freonIn = round(getTemperature(freonInPin) * 10.0) / 10.0;
        temp_freonOut = round(getTemperature(freonOutPin) * 10.0) / 10.0;
        temp_heatExchange = round(getTemperature(heatExchangeTempPin) * 10.0) / 10.0;
        temp_outside = round(getTemperature(outsideTempPin) * 10.0) / 10.0;


        //temp_outside = sensors.getTempCByIndex(0);

        //sensors.requestTemperatures();
        tempCheckStartTime = millis();
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
    reading = SERIESRESISTOR * reading;  // 10K / (1023/ADC - 1) //if resistor is connected to ground
    //reading = SERIESRESISTOR * reading; //if resistor is connected to 5V

    float steinhart;
    steinhart = reading / THERMISTORNOMINAL;     // (R/Ro)
    steinhart = log(steinhart);                  // ln(R/Ro)
    steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                 // Invert
    steinhart -= 273.15;                         // convert to C

    return steinhart;
}

void resetTimes()
{
    long time = millis();

    flowRateCheckStartTime = time;
    startTime = time;
    tempCheckStartTime = time;
}
