#include <LiquidCrystal_I2C.h>
#include "Display.h"

const byte LineLength = 20;
const byte NumberOfLines = 4;

LiquidCrystal_I2C lcd(0x27, LineLength, NumberOfLines);

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
    char s[LineLength + sizeof(char)];
    sprintf(s, "%2u.%02u", speedA, speedB);
    lcd.setCursor(15, 0);
    lcd.print(s);
}

void PrintDShotLine(const unsigned short speedDShot)
{
    char str[LineLength + sizeof(char)];
    sprintf(str, "DShot: %u", speedDShot);

    lcd.setCursor(0, 1);
    lcd.print(str);
}

void ResetDShotLine()
{
    lcd.setCursor(0, 1);
    lcd.print("                    ");
}

void PrintBallInfoLine(const String message)
{
    lcd.setCursor(0, 1);

    byte numberOfLeadingSpaces = (LineLength - message.length()) / 2;
    byte numberOfTrailingSpaces = (LineLength - message.length()) - numberOfLeadingSpaces;
    for (byte i = 0; i < numberOfLeadingSpaces; i++)
    {
        lcd.print(" ");
    }

    lcd.print(message);

    for (byte i = 0; i < numberOfTrailingSpaces; i++)
    {
        lcd.print(" ");
    }
}

void ResetBallInfoLine()
{
    lcd.setCursor(0, 1);
    lcd.print("                    ");
}

void PrintStatusLine(const String message)
{
    char str[LineLength + sizeof(char)];
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

        byte numberOfSpaces = (LineLength / 2) - 2 - buttonLabel.length();
        for (byte i = 0; i < numberOfSpaces; i++)
        {
            lcd.print(" ");
        }
    }
}

void PrintRightButton(const String buttonLabel)
{
    lcd.setCursor((LineLength / 2), 3);

    if (buttonLabel.length() == 0)
    {
        lcd.print("          ");
    }
    else
    {
        byte numberOfSpaces = (LineLength / 2) - 2 - buttonLabel.length();
        for (byte i = 0; i < numberOfSpaces; i++)
        {
            lcd.print(" ");
        }

        lcd.print("[");
        lcd.print(buttonLabel);
        lcd.print("]");
    }
}
