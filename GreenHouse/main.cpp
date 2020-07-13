/*
lcd.backlight() - turn on backlight
lcd.noBacklight() - turn off
lcd.setCursor(0,0);
lcd.print();
lcd.clear();

Modes:
    'h' - home
    'm' - change mode
    'x' - set max temperature
    'n' - set min temperature

EEPROM:

    6 - min temp
    1 - min temp decimal
    2 - max temp
    3 - max temp decimal
    4 - isAuto
    5 - isOpened


*/
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

#define ONE_WIRE_BUS 2

#define upButton 6
#define downButton 7
#define okButton 5
#define modeButton 4

#define minTempAddress 6
#define maxTempAddress 2
#define minTempDecimalAddress 1
#define maxTempDecimalAddress 3
#define isAutoAddress 4
#define isOpenedAddress 5

#define relay1 12
#define relay2 11
#define relay3 10
#define relay4 9

#define max_time 100000


OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

bool isAuto = true;
bool isOpened = false;
long startTime;
long tempCheckStartTime;
long lightDelayTime = 30000;
long screenDelayTime = 60000;
long tempCheckDelayTime = 5000;
bool backlight = false;
float temp = 23.9;

long relay1StartTime;
long relay2StartTime;
long relayDelayTime = 100000;

float minTemp = 24;
float maxTemp = 26;

bool isUpButtonPressed = false;
bool isDownButtonPressed = false;
bool isOkButtonPressed = false;
bool isModeButtonPressed = false;
char mode = 'h';

void resetTimes();
void readButtons();
void checkBacklight();
void homeScreen();
void modeScreen();
void changeMinTempScreen();
void changeMaxTempScreen();
void automatedWindows();
void checkTemp();
void checkScreen();

void closeWindows();
void openWindows();
bool areWindowsOpening();
bool windowsOpening = false;

void saveMinTemp();
void saveMaxTemp();



void setup()
{
  pinMode(upButton, INPUT);
  pinMode(downButton, INPUT);
  pinMode(okButton, INPUT);
  pinMode(modeButton, INPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  Serial.begin(9600);
  sensors.begin();
  lcd.begin(16,2);

  lcd.noBacklight();

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);

  isAuto = EEPROM.read(isAutoAddress);
  isOpened = EEPROM.read(isOpenedAddress);
  float minDecimal = (float)EEPROM.read(minTempDecimalAddress)/10;
  float maxDecimal = (float)EEPROM.read(maxTempDecimalAddress)/10;

  minTemp = (float)EEPROM.read(minTempAddress) + minDecimal + 0.001;
  maxTemp = (float)EEPROM.read(maxTempAddress) + maxDecimal + 0.001;

  checkTemp();
  tempCheckStartTime = millis();


}
void loop(void)
{
    long begginingTime = millis();

    checkTemp();
    readButtons();
    checkBacklight();
    checkScreen();

    if(mode == 'h'){
        home();
    }else if(mode == 'm'){
        modeScreen();
    }else if(mode == 'x'){
        changeMaxTempScreen();
    }else if(mode == 'n'){
        changeMinTempScreen();
    }

    if(isAuto){
        automatedWindows();
    }

    if(!areWindowsOpening()){
        digitalWrite(relay1, HIGH);
        digitalWrite(relay2, HIGH);
        delay(50);
        digitalWrite(relay3, HIGH);
    }


    if(millis()-begginingTime > 100000){
      resetTimes();
    }



}

void saveMaxTemp(){
    int num = maxTemp;
    int decimal = (maxTemp - num)*10;
    EEPROM.write(maxTempAddress, num);
    EEPROM.write(maxTempDecimalAddress, decimal);
}

void saveMinTemp(){
    int num = minTemp;
    int decimal = (minTemp - num)*10;
    EEPROM.write(minTempAddress, num);
    EEPROM.write(minTempDecimalAddress, decimal);
}

void resetTimes(){
    long time = millis();

    relay1StartTime = time;
    relay2StartTime = time;
    startTime = time;
    tempCheckStartTime = time;
}

bool areWindowsOpening(){
    if(relay1StartTime + relayDelayTime <= millis() && relay2StartTime + relayDelayTime <= millis()){
        return false;
    }else{
        return true;
    }
}

void openWindows(){
    isOpened = true;
    EEPROM.write(isOpenedAddress, true);
    digitalWrite(relay3, LOW);
    delay(50);
    digitalWrite(relay2, HIGH);
    digitalWrite(relay1, LOW);
    relay1StartTime = millis();
}

void closeWindows(){
    isOpened = false;
    EEPROM.write(isOpenedAddress, false);
    digitalWrite(relay3, LOW);
    delay(50);
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, LOW);
    relay2StartTime = millis();
}

void automatedWindows(){ // function for auto mode
    if(temp > maxTemp && isOpened == false){
        openWindows();
    }else if(temp < minTemp && isOpened == true){
        closeWindows();
    }

}

void home(){

    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.print(temp, 1);


    if(isAuto){
        lcd.print("  auto");
    }else{
        lcd.print(" ru4no");
    }

    lcd.setCursor(0,1);

    if(isOpened){
        lcd.print("Otvoreni");
    }else{
        lcd.print("Zatvoreni");
    }



    if(isUpButtonPressed){
        while(isUpButtonPressed){
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            if(!isAuto && isOpened == false){
                openWindows();
                lcd.clear();
            }
        }else{
            lcd.backlight();
            backlight = true;
        }


    }else if(isDownButtonPressed){
        while(isDownButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            if(!isAuto && isOpened == true){
                closeWindows();
                lcd.clear();
            }
        }else{
            lcd.backlight();
            backlight = true;
        }

    }else if(isOkButtonPressed){
        while(isOkButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){

        }else{
            lcd.backlight();
            backlight = true;
        }

    }else if(isModeButtonPressed){
        while(isModeButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            mode = 'm';
            lcd.clear();
        }else{
            lcd.backlight();
            backlight = true;
        }
    }
}

void modeScreen(){
    lcd.setCursor(0, 0);

    if(isAuto){
        lcd.print("smeni na ruchen");

    }else {
        lcd.print("smeni na auto");
    }

    if(isUpButtonPressed){
        while(isUpButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){

        }else{
            lcd.backlight();
            backlight = true;
        }


    }else if(isDownButtonPressed){
        while(isDownButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){

        }else{
            lcd.backlight();
            backlight = true;
        }

    }else if(isOkButtonPressed){
        while(isOkButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){

            isAuto = !isAuto;
            EEPROM.write(isAutoAddress, isAuto);
            lcd.clear();
            mode = 'h';

        }else{
            lcd.backlight();
            backlight = true;
        }

    }else if(isModeButtonPressed){
        while(isModeButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            mode = 'n';
            lcd.clear();
        }else{
            lcd.backlight();
            backlight = true;
        }
    }
}

void changeMinTempScreen(){

    lcd.setCursor(0,0);
    lcd.print("min temp: ");
    lcd.print(minTemp, 1);

    if(isUpButtonPressed){
        while(isUpButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            minTemp = minTemp + 0.1;
            saveMinTemp();
        }else{
            lcd.backlight();
            backlight = true;
        }


    }else if(isDownButtonPressed){
        while(isDownButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            minTemp = minTemp - 0.1;
            saveMinTemp();
        }else{
            lcd.backlight();
            backlight = true;
        }

    }else if(isOkButtonPressed){
        while(isOkButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            lcd.clear();
            mode = 'h';
        }else{
            lcd.backlight();
            backlight = true;
        }

    }else if(isModeButtonPressed){
        while(isModeButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            mode = 'x';
            lcd.clear();
        }else{
            lcd.backlight();
            backlight = true;
        }
    }

}

void changeMaxTempScreen(){

    lcd.setCursor(0,0);
    lcd.print("max temp: ");
    lcd.print(maxTemp, 1);

    if(isUpButtonPressed){
        while(isUpButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            maxTemp = maxTemp + 0.1;
            saveMaxTemp();
        }else{
            lcd.backlight();
            backlight = true;
        }


    }else if(isDownButtonPressed){
        while(isDownButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            maxTemp = maxTemp - 0.1;
            saveMaxTemp();
        }else{
            lcd.backlight();
            backlight = true;
        }

    }else if(isOkButtonPressed){
        while(isOkButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            lcd.clear();
            mode = 'h';
        }else{
            lcd.backlight();
            backlight = true;
        }

    }else if(isModeButtonPressed){
        while(isModeButtonPressed){
          //wait
          readButtons();
        }
        startTime = millis();

        if(backlight == true){
            mode = 'h';
            lcd.clear();
        }else{
            lcd.backlight();
            backlight = true;
        }
    }
}

void readButtons(){
    isUpButtonPressed = digitalRead(upButton);
    isDownButtonPressed = digitalRead(downButton);
    isOkButtonPressed = digitalRead(okButton);
    isModeButtonPressed = digitalRead(modeButton);

}

void checkBacklight(){
    if(backlight == true){
        if(startTime + lightDelayTime <= millis()){
            lcd.noBacklight();
            backlight = false;
        }
    }
}

void checkTemp(){
    if(tempCheckStartTime + tempCheckDelayTime < millis()){
        sensors.requestTemperatures(); // Send the command to get temperature readings
        temp = sensors.getTempCByIndex(0);
        tempCheckStartTime = millis();
    }
}

void checkScreen(){

    if(startTime + screenDelayTime <= millis()){
        mode = 'h';
    }

}
