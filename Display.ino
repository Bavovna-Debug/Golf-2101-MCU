#include <LiquidCrystal_I2C.h>
#include "Display.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);

void InitDisplay()
{
    lcd.init();
    lcd.backlight();
}

void ResetDisplay()
{
    lcd.setCursor(0, 0);
    lcd.print("Shoot strength");
}

void DisplaySpeed(const unsigned short speedA, const unsigned short speedB)
{
    char s[20];
    sprintf(s, "%2u.%02u", speedA, speedB);
    lcd.setCursor(15, 0);
    lcd.print(s);
}

void PrintDShotLine(const unsigned short speedDShot)
{
    char str[21];
    sprintf(str, "DShot: %u", speedDShot);

    lcd.setCursor(0, 1);
    lcd.print(str);
}

void ResetDShotLine()
{
    lcd.setCursor(0, 1);
    lcd.print("                    ");
}

void PrintStatusLine(const String message)
{
    char str[21];
    sprintf(str, "Debug: %s", message.c_str());
    str[sizeof(str) - sizeof(char)] = 0;

    lcd.setCursor(0, 2);
    lcd.print(str);
}

void ResetStatusLine()
{
    lcd.setCursor(0, 2);
    lcd.print("                    ");
}

void PrintLeftButton(const String buttonLabel)
{
    lcd.setCursor(0, 3);

    if (buttonLabel.length() == 0)
    {
        lcd.print("          ");
    }
    else
    {
        lcd.print("[");
        lcd.print(buttonLabel);
        lcd.print("]");

        byte numberOfSpaces = (10 * sizeof(char)) - (2 * sizeof(char)) - buttonLabel.length();
        for (byte i = 0; i < numberOfSpaces; i++)
        {
            lcd.print(" ");
        }
    }
}

void PrintRightButton(const String buttonLabel)
{
    lcd.setCursor(10, 3);

    if (buttonLabel.length() == 0)
    {
        lcd.print("          ");
    }
    else
    {
        byte numberOfSpaces = (10 * sizeof(char)) - (2 * sizeof(char)) - buttonLabel.length();
        for (byte i = 0; i < numberOfSpaces; i++)
        {
            lcd.print(" ");
        }

        lcd.print("[");
        lcd.print(buttonLabel);
        lcd.print("]");
    }
}
