#include <Wire.h>
#include <EEPROM.h>
#include <Encoder.h>
#include "API.h"
#include "Display.h"
#include "DShot.h"

const int PinRotaryAL   = 2;
const int PinRotaryAR   = 3;
const int PinRotaryBL   = 6;
const int PinRotaryBR   = 7;
const int PinButtonL    = 4;
const int PinButtonR    = 5;
const int PinESC        = 8;

const unsigned long DelayAfterSpeedUp = 2000lu;
const unsigned long FlashSettingsInterval = 10lu * 1000lu;

Encoder encoderA(PinRotaryAL, PinRotaryAR);
Encoder encoderB(PinRotaryBL, PinRotaryBR);

DShot esc;

static bool constantMotorMode = false;

struct EEPROMData
{
    unsigned long contentVersion;
    unsigned short speedA;
    unsigned short speedB;
    unsigned int speedDisplayed;
};

EEPROMData eepromData;

static unsigned long contentVersion = 0;
static unsigned long schedulerEEPROM = 0;

long lastA = 0;
long lastB = 0;

unsigned short speedA = 0;
unsigned short speedB = 0;
signed short speedDisplayed = 0;
unsigned short speedDShot = 0;

void setup()
{
    Wire.begin();

    pinMode(PinButtonL, INPUT);
    pinMode(PinButtonR, INPUT);

    esc.attach(PinESC);
    esc.setThrottle(0);

    InitDisplay();

    LoadFromEEPROM();

    ValidateSpeed();

    ResetDisplay();
    DisplaySpeed(speedA, speedB);

    PrintLeftButton("MTR STRT");
    PrintRightButton("SHOOT");
}

void loop()
{
    unsigned long timestamp = millis();

    // Periodically update current values in EEPROM.
    //
    if (timestamp > schedulerEEPROM)
    {
        schedulerEEPROM = timestamp + FlashSettingsInterval;
        StoreToEEPROM();
    }

    long currentA = encoderA.read() / 4;
    long currentB = encoderB.read() / 4;
    if ((currentA != lastA) || (currentB != lastB))
    {
        if (currentA < lastA) speedA--;
        if (currentA > lastA) speedA++;
        if (currentB < lastB) speedB--;
        if (currentB > lastB) speedB++;

        lastA = currentA;
        lastB = currentB;

        ValidateSpeed();

        DisplaySpeed(speedA, speedB);

        contentVersion++;
    }

    if (LeftButtonPressed() == true)
    {
        if (constantMotorMode == false)
        {
            constantMotorMode = true;
            PrintLeftButton("");
            StartMotor();
            PrintLeftButton("MTR STOP");
        }
        else
        {
            constantMotorMode = false;
            PrintLeftButton("");
            StopMotor();
            PrintLeftButton("MTR STRT");
        }
    }

    if (RightButtonPressed() == true)
    {
        Fire();
    }
}

void ValidateSpeed()
{
    speedDisplayed = (speedA * 100) + speedB;

    if (speedDisplayed < 0)
    {
        speedDisplayed = 0;
    }

    if (speedDisplayed > 2000)
    {
        speedDisplayed = 2000;
    }

    speedA = speedDisplayed / 100;
    speedB = speedDisplayed % 100;

    speedDShot = speedDisplayed + 48;
}

bool LeftButtonPressed()
{
    return (digitalRead(PinButtonL) == LOW);
}

bool RightButtonPressed()
{
    return (digitalRead(PinButtonR) == LOW);
}

void StartMotor()
{
    PrintStatusLine("speed up");

    for (unsigned short currentSpeed = 100;
         currentSpeed < speedDShot;
         currentSpeed++)
    { 
        esc.setThrottle(currentSpeed);
        delay(1);
    }

    ResetStatusLine();
}

void StopMotor()
{
    PrintStatusLine("slow down");

    for (unsigned short currentSpeed = speedDShot;
         currentSpeed > 100;
         currentSpeed--)
    { 
        esc.setThrottle(currentSpeed);
        delay(1);
    }

    esc.setThrottle(0);

    ResetStatusLine();
}

void Fire()
{
    if (constantMotorMode == false)
    {
        StartMotor();

        PrintDShotLine(speedDShot);

        PrintStatusLine("wait for RPM");
        delay(DelayAfterSpeedUp);
        ResetStatusLine();

    }

    do
    {
        PrintStatusLine("fire");

        Wire.beginTransmission(8);
        Wire.write(byte(SERVO_START));
        Wire.endTransmission();

        for (;;)
        {
            Wire.requestFrom(8, 1);
            while (!Wire.available()) { }
            byte statusByte = Wire.read();

            if (statusByte == SERVO_LOAD)
            {
                PrintStatusLine("ball load");
            }
            else if (statusByte == SERVO_WAIT)
            {
                PrintStatusLine("delay");
            }
            else if (statusByte == SERVO_TURN)
            {
                PrintStatusLine("ball turn");
            }
            else if (statusByte == SERVO_DONE)
            {
                ResetStatusLine();
                break;
            }

            delay(100);
        }
    }
    while (RightButtonPressed() == true);

    if (constantMotorMode == false)
    {
        ResetDShotLine();

        StopMotor();
    }
}

void LoadFromEEPROM()
{
    EEPROM.get(0x00, eepromData);

    // On a factory new device with "empty" EEPROM the contents of EEPROM are set to 0xFF.
    // If a value of 0xFF is read (which is unexpected for channel number) then we assume
    // that values were never stored in EEPROM yet.
    //
    if (eepromData.contentVersion == 0xFFFFFFFF)
    {
        memset(&eepromData, 0x00, sizeof(eepromData));
    }
    else
    {
        contentVersion = eepromData.contentVersion;
        speedA = eepromData.speedA;
        speedB = eepromData.speedB;
        speedDisplayed = eepromData.speedDisplayed;
    }
}

void StoreToEEPROM()
{
    // Write to EEPROM only if user did change values since last EEPROM update.
    //
    if (contentVersion != eepromData.contentVersion)
    {
        eepromData.contentVersion = contentVersion;
        eepromData.speedA = speedA;
        eepromData.speedB = speedB;
        eepromData.speedDisplayed = speedDisplayed;
        EEPROM.put(0x00, eepromData);
    }
}
