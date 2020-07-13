#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

#define button 4
#define numberOfScreens 2

bool isButtonPressed = false;
bool wasButtonHold = false;
bool backlight = false;
long startTime;
long lightDelayTime = 3000;
int screen = 1;

void setup(void)
{
    Serial.begin(9600);

    lcd.begin(16,2);
    lcd.noBacklight();
    pinMode(button, INPUT);
}

void loop(void)
{
    readButton();
    checkBacklight();

    switch(screen)
    {
        case 1: printTempScreen();
            break;
        case 2: printFlowScreen();
            break;
    }


}

void printTempScreen()
{

    lcd.setCursor(0,0);
    lcd.print("Temperature");
    lcd.setCursor(0,1);
    lcd.print("Screen");
}

void printFlowScreen()
{

    lcd.setCursor(0,0);
    lcd.print("Flow Rate");
    lcd.setCursor(0,1);
    lcd.print("Screen");
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

        if(backlight == false)
        {
            lcd.backlight();
            backlight = true;
        }
        else
        {
            if(screen >= numberOfScreens)
            {
                screen = 1;
            }
            else
            {
                screen++;
            }
            lcd.clear();
        }

    }
}

void checkBacklight()
{
    if(backlight == true)
    {
        if(startTime + lightDelayTime <= millis())
        {
            lcd.noBacklight();
            backlight = false;
        }
    }
}

