#define hallSensorInterrupt 0  // 0 = digital pin 2
#define hallSensorPin 2

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 6.0;

volatile byte pulseCount;

float flowRate;

unsigned long flowRateCheckStartTime;

void setup()
{

  // Initialize a serial connection for reporting values to the host
  Serial.begin(9600);

  pinMode(hallSensorPin, INPUT);
  digitalWrite(hallSensorPin, HIGH);

  pulseCount = 0;
  flowRate = 0.0;
  flowRateCheckStartTime = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(hallSensorInterrupt, pulseCounter, FALLING);
}

/**
 * Main program loop
 */
void loop()
{
   checkFlowRate();

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
