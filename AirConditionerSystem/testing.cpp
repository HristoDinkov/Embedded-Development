void checkTemp()
{
    if(tempCheckStartTime + tempCheckDelayTime < millis())
    {
        temp_waterIn = round(getTemperature(waterInPin) * 10.0) / 10.0;
        temp_waterOut = round(getTemperature(waterOutPin) * 10.0) / 10.0;
        temp_freonIn = round(getTemperature(freonInPin) * 10.0) / 10.0;
        temp_freonOut = round(getTemperature(freonOutPin) * 10.0) / 10.0;
        temp_heatExchange = round(getTemperature(heatExchangeTempPin) * 10.0) / 10.0;

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
    reading = SERIESRESISTOR / reading;  // 10K / (1023/ADC - 1)

    float steinhart;
    steinhart = reading / THERMISTORNOMINAL;     // (R/Ro)
    steinhart = log(steinhart);                  // ln(R/Ro)
    steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                 // Invert
    steinhart -= 273.15;                         // convert to C

    return steinhart;
}

Serial.print(temp_waterIn);
Serial.print(" | ");
Serial.print(temp_waterOut);
Serial.print(" | ");
Serial.print(temp_freonIn);
Serial.print(" | ");
Serial.print(temp_freonOut);
Serial.print(" | ");
Serial.print(temp_heatExchange);
Serial.println(" | ");





