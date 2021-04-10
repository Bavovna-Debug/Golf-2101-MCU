#include <LiquidCrystal_I2C.h>
#include "Display.h"

const byte LineLength = 20;
const byte NumberOfLines = 4;

LiquidCrystal_I2C lcd(0x27, LineLength, NumberOfLines);

void InitDisplay(void)
{
    lcd.init();
    lcd.backlight();
}

void ResetDisplay(void)
{
}

void PrintBatteryStatus(const byte batteryLevel)
{
    if (batteryLevel == 0)
    {
        lcd.setCursor(16, 0);
        lcd.print("Low!");
    }
    else
    {
        char s[LineLength + sizeof(char)];
        sprintf(s, "%3u%%", batteryLevel);
        lcd.setCursor(16, 0);
        lcd.print(s);
    }
}

void PrintDebugLine(const String message)
{
    lcd.setCursor(0, 3);

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

void DisplaySpeed(const unsigned short speedA, const unsigned short speedB, const bool debugMode)
{
    if (debugMode == false)
    {
        char s[LineLength + sizeof(char)];
        sprintf(s, "Shoot %2u.%02u", speedA, speedB);
        lcd.setCursor(0, 0);
        lcd.print(s);
    }
    else
    {
        char s[LineLength + sizeof(char)];
        sprintf(s, "DShot %2u.%02u DBG", speedA, speedB);
        lcd.setCursor(0, 0);
        lcd.print(s);
    }
}

void PrintDShotLine(const unsigned short speedDShot)
{
    char str[LineLength + sizeof(char)];
    sprintf(str, "DShot: %u", speedDShot);

    lcd.setCursor(0, 1);
    lcd.print(str);
}

void ResetDShotLine(void)
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

void ResetBallInfoLine(void)
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

void ResetStatusLine(void)
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
