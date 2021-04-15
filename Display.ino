#include <LiquidCrystal_I2C.h>
#include "Battery.h"
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

void PrintInfoLine1(const String message)
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

void ResetInfoLine1(void)
{
    lcd.setCursor(0, 1);
    lcd.print("                    ");
}

void PrintInfoLine2(const String message)
{
    lcd.setCursor(0, 2);

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

void ResetInfoLine2(void)
{
    lcd.setCursor(0, 2);
    lcd.print("                    ");
}

void PrintDebugLine(const String message)
{
    char str[LineLength + sizeof(char)];
    sprintf(str, "Debug: %s", message.c_str());
    str[sizeof(str) - sizeof(char)] = 0;

    lcd.setCursor(0, 2);
    lcd.print(str);
}

void ResetDebugLine(void)
{
    lcd.setCursor(0, 2);
    lcd.print("                    ");
}

void PrintSpecialLine(const String message)
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

void BatteryInit(void)
{
    lcd.setCursor(16, 0);
    lcd.write(byte(0));
    lcd.write(byte(1));
    lcd.write(byte(2));
    lcd.write(byte(3));
}

void BatteryPrint(const byte level)
{
    if (level < 10)
    {
        lcd.createChar(0, SymbolBatteryA[0]);
        lcd.createChar(1, SymbolBatteryB[0]);
        lcd.createChar(2, SymbolBatteryB[0]);
        lcd.createChar(3, SymbolBatteryC[0]);
    }
    else if (level < 25)
    {
        lcd.createChar(0, SymbolBatteryA[1]);
        lcd.createChar(1, SymbolBatteryB[0]);
        lcd.createChar(2, SymbolBatteryB[0]);
        lcd.createChar(3, SymbolBatteryC[0]);
    }
    else if (level < 40)
    {
        lcd.createChar(0, SymbolBatteryA[1]);
        lcd.createChar(1, SymbolBatteryB[1]);
        lcd.createChar(2, SymbolBatteryB[0]);
        lcd.createChar(3, SymbolBatteryC[0]);
    }
    else if (level < 55)
    {
        lcd.createChar(0, SymbolBatteryA[1]);
        lcd.createChar(1, SymbolBatteryB[2]);
        lcd.createChar(2, SymbolBatteryB[0]);
        lcd.createChar(3, SymbolBatteryC[0]);
    }
    else if (level < 70)
    {
        lcd.createChar(0, SymbolBatteryA[1]);
        lcd.createChar(1, SymbolBatteryB[2]);
        lcd.createChar(2, SymbolBatteryB[1]);
        lcd.createChar(3, SymbolBatteryC[0]);
    }
    else if (level < 85)
    {
        lcd.createChar(0, SymbolBatteryA[1]);
        lcd.createChar(1, SymbolBatteryB[2]);
        lcd.createChar(2, SymbolBatteryB[2]);
        lcd.createChar(3, SymbolBatteryC[0]);
    }
    else
    {
        lcd.createChar(0, SymbolBatteryA[1]);
        lcd.createChar(1, SymbolBatteryB[2]);
        lcd.createChar(2, SymbolBatteryB[2]);
        lcd.createChar(3, SymbolBatteryC[1]);
    }
}
