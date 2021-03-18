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

// Value of 582 corresponds to 8.4V by a resistor bridge of 10 kOhm / 5.1 kOhm.
//
const int BatteryLowLevel = 582;

const unsigned long DelayAfterSpeedUp = 2000lu;
const unsigned long FlashSettingsInterval = 10lu * 1000lu;
const unsigned long BatteryStatusInterval = 5lu * 1000lu;
const unsigned long BatteryInitializationMessageDelay = 5lu * 1000lu;

Encoder encoderA(PinRotaryAL, PinRotaryAR);
Encoder encoderB(PinRotaryBL, PinRotaryBR);

DShot esc;

static bool constantMotorMode = false;

struct EEPROMData
{
    unsigned long contentVersion;
    int batteryFullLevel;
    unsigned short speedA;
    unsigned short speedB;
    unsigned int speedDisplayed;
};

EEPROMData eepromData;

static unsigned long contentVersion = 0;
static unsigned long schedulerEEPROM = 0;
static unsigned long schedulerBattery = 0;

long lastA = 0;
long lastB = 0;

int batteryFullLevel = 0;
unsigned short speedA = 0;
unsigned short speedB = 0;
signed short speedDisplayed = 0;
unsigned short speedDShot = 0;
bool ballAvailableState = true;

void setup()
{
    Wire.begin();

    pinMode(PinButtonL, INPUT);
    pinMode(PinButtonR, INPUT);

    esc.attach(PinESC);
    esc.setThrottle(0);

    InitDisplay();

    LoadFromEEPROM();

    ResetDisplay();

    InitializeBatteryFullLevel();

    ResetDisplay();

    ValidateSpeed();
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
        StoreToEEPROM(false);
    }

    // Periodically update the battery status line.
    //
    if (timestamp > schedulerBattery)
    {
        schedulerBattery = timestamp + BatteryStatusInterval;
        UpdateBatteryStatus();
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

    bool ballAvailableStateNew = IsBallAvailable();
    if (ballAvailableStateNew != ballAvailableState)
    {
        ballAvailableState = ballAvailableStateNew;

        if (ballAvailableState == false)
        {
            PrintBallInfoLine("Load a ball!");
            PrintRightButton("");
        }
        else
        {
            ResetBallInfoLine();
            PrintRightButton("SHOOT");
        }
    }

    if (IsLeftButtonPressed() == true)
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

    if ((IsRightButtonPressed() == true) && (ballAvailableState == true))
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

    if (constantMotorMode == true)
    {
        esc.setThrottle(speedDShot);
    }
}

bool IsLeftButtonPressed()
{
    return (digitalRead(PinButtonL) == LOW);
}

bool IsRightButtonPressed()
{
    return (digitalRead(PinButtonR) == LOW);
}

bool IsBallAvailable()
{
    Wire.requestFrom(8, 1);
    while (!Wire.available()) { }
    byte payload = Wire.read();
    return ((payload & BALL_STATE) == BALL_STATE);
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

void StartServo()
{
    Wire.beginTransmission(8);
    Wire.write(byte(SERVO_START));
    Wire.endTransmission();
}

byte FetchServoStatus()
{
    Wire.requestFrom(8, 1);
    while (!Wire.available()) { }
    byte payload = Wire.read();
    return (payload & SERVO_MASK);
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

        StartServo();

        byte servoStatus = 0;
        do
        {
            byte servoStatusNew = FetchServoStatus();

            if (servoStatusNew != servoStatus)
            {
                servoStatus = servoStatusNew;

                switch (servoStatus)
                {
                    case SERVO_LOAD:
                        PrintStatusLine("ball load");
                        break;

                    case SERVO_TURN:
                        PrintStatusLine("ball turn");
                        break;

                    case SERVO_DONE:
                        ResetStatusLine();
                        break;
                }

                delay(100);
            }
        }
        while (servoStatus != SERVO_DONE);
    }
    while (IsRightButtonPressed() == true);

    if (constantMotorMode == false)
    {
        ResetDShotLine();

        StopMotor();
    }
}

void InitializeBatteryFullLevel()
{
    if ((IsLeftButtonPressed() == true) && (IsLeftButtonPressed() == true))
    {
        PrintDebugLine("Initializing battery");

        delay(BatteryInitializationMessageDelay);

        if ((IsLeftButtonPressed() == true) && (IsLeftButtonPressed() == true))
        {
            batteryFullLevel = analogRead(A3);
            StoreToEEPROM(true);

            PrintDebugLine("New level saved");

            delay(1000);

            PrintDebugLine("Release buttons");
        }
        else
        {
            PrintDebugLine("Cancelled");
        }

        while ((IsLeftButtonPressed() == true) || (IsLeftButtonPressed() == true)) { }

        delay(1000);
    }
}

void UpdateBatteryStatus()
{
    signed int batteryLevelInPercent;

    if (batteryFullLevel == 0)
    {
        batteryLevelInPercent = 0;
    }
    else
    {
        int batteryCurrentLevel = analogRead(A3);
        batteryLevelInPercent = map(batteryCurrentLevel, BatteryLowLevel, batteryFullLevel, 0, 100);
    }

    if (batteryLevelInPercent < 0)
    {
        batteryLevelInPercent = 0;
    }
    if (batteryLevelInPercent > 100)
    {
        batteryLevelInPercent = 100;
    }

    PrintBatteryStatus(batteryLevelInPercent);
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
        batteryFullLevel = eepromData.batteryFullLevel;
        speedA = eepromData.speedA;
        speedB = eepromData.speedB;
        speedDisplayed = eepromData.speedDisplayed;
    }
}

void StoreToEEPROM(const bool forced)
{
    if (forced == true)
    {
        contentVersion++;
    }

    // Write to EEPROM only if user did change values since last EEPROM update.
    //
    if (contentVersion != eepromData.contentVersion)
    {
        eepromData.contentVersion = contentVersion;
        eepromData.batteryFullLevel = batteryFullLevel;
        eepromData.speedA = speedA;
        eepromData.speedB = speedB;
        eepromData.speedDisplayed = speedDisplayed;
        EEPROM.put(0x00, eepromData);
    }
}
