#include <Wire.h>
#include <EEPROM.h>
#include <Encoder.h>
#include "API.h"
#include "Display.h"
#include "DShot.h"

const int PinRotaryAL   = 11;
const int PinRotaryAR   = 10;
const int PinRotaryBL   = 3;
const int PinRotaryBR   = 2;
const int PinButtonL    = 5;
const int PinButtonR    = 4;
const int PinESC        = 8;

// Value of 582 corresponds to 8.4V by a resistor bridge of 10 kOhm / 5.1 kOhm.
//
const int BatteryLowLevel = 582;

// Speed limits for normal mode and debug mode.
//
const signed short shootSpeedMinimumNormalMode  = 100;
const signed short shootSpeedMaximumNormalMode  = 1200;
const signed short shootSpeedMinimumDebugMode   = 0;
const signed short shootSpeedMaximumDebugMode   = 2047;

// This value will be added to the shoot value in a non-debug mode.
//
const signed short DShotDeltaNormalMode         = 40;
const signed short DShotDeltaDebugMode          = 0;

const unsigned long MotorSpeedUpDelay                   = 2lu;
const unsigned long MotorSlowDownDelay                  = 1lu;
const unsigned long DelayAfterSpeedUp                   = 2lu * 1000lu;
const unsigned long FlashSettingsInterval               = 5lu * 1000lu;
const unsigned long BatteryStatusInterval               = 5lu * 1000lu;
const unsigned long IdleMotorWarningInterval            = 10lu * 60lu * 1000lu; // 10 minutes.
const unsigned long EnteringSetupModeMessageDelay       = 2lu * 1000lu;
const unsigned long LeavingSetupModeMessageDelay        = 2lu * 1000lu;

Encoder encoderA(PinRotaryAL, PinRotaryAR);
Encoder encoderB(PinRotaryBL, PinRotaryBR);
DShot esc;

struct EEPROMData
{
    unsigned long contentVersion;
    int batteryFullLevel;
    unsigned short speedA;
    unsigned short speedB;
};

EEPROMData eepromData;

static unsigned long contentVersion = 0;
static unsigned long schedulerEEPROM = 0;
static unsigned long schedulerBattery = 0;
static unsigned long schedulerIdleMotor = IdleMotorWarningInterval;

static long lastA = 0;
static long lastB = 0;

static int batteryFullLevel = 0;
static unsigned short speedA = 0;
static unsigned short speedB = 0;
static signed short speedDisplayed = 0;
static unsigned short speedDShot = 0;
static signed short shootSpeedMinimum = shootSpeedMinimumNormalMode;
static signed short shootSpeedMaximum = shootSpeedMaximumNormalMode;
static signed short dshotDelta = DShotDeltaNormalMode;
static bool debugMode = false;
static bool motorRunning = false;
static bool permanentMotorRun = false;
static bool ballAvailableState = true;

void setup(void)
{
    Wire.begin();

    pinMode(PinButtonL, INPUT);
    pinMode(PinButtonR, INPUT);

    esc.attach(PinESC);
    esc.setThrottle(0);

    InitDisplay();

    LoadFromEEPROM();

    ResetDisplay();

    ProcessSetupMode();

    ResetDisplay();
    BatteryInit();

    ValidateSpeed();
    DisplaySpeed(speedA, speedB, debugMode);

    PrintLeftButton("MTR STRT");
    PrintRightButton("SHOOT");
}

void loop(void)
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

    // Check whether ESC is for too long time idle. Give a warning before ESC begins to beep.
    //
    if (timestamp > schedulerIdleMotor)
    {
        schedulerIdleMotor = timestamp + IdleMotorWarningInterval;
        if (motorRunning == false)
        {
            IdleMotorWarning();
        }
    }

    long currentA = encoderA.read() / 4;
    long currentB = encoderB.read() / 4;
    if ((currentA != lastA) || (currentB != lastB))
    {
        schedulerEEPROM = timestamp + FlashSettingsInterval;

        if (currentA < lastA) speedA--;
        if (currentA > lastA) speedA++;
        if (currentB < lastB) speedB--;
        if (currentB > lastB) speedB++;

        lastA = currentA;
        lastB = currentB;

        ValidateSpeed();

        DisplaySpeed(speedA, speedB, debugMode);

        contentVersion++;
    }

    bool ballAvailableStateNew = IsBallAvailable();
    if (ballAvailableStateNew != ballAvailableState)
    {
        ballAvailableState = ballAvailableStateNew;

        if (ballAvailableState == false)
        {
            PrintInfoLine1("Load a ball!");
            if (debugMode == false)
            {
                PrintRightButton("");
            }
        }
        else
        {
            ResetInfoLine1();
            PrintRightButton("SHOOT");
        }
    }

    if (IsLeftButtonPressed() == true)
    {
        while (IsLeftButtonPressed() == true) { }

        if (permanentMotorRun == false)
        {
            PrintLeftButton("");
            StartMotor(true);
            PrintLeftButton("MTR STOP");
        }
        else
        {
            PrintLeftButton("");
            StopMotor();
            PrintLeftButton("MTR STRT");

            schedulerIdleMotor = timestamp + IdleMotorWarningInterval;
        }
    }

    if (IsRightButtonPressed() == true)
    {
        if (ballAvailableState == true)
        {
            Fire();

            schedulerIdleMotor = timestamp + IdleMotorWarningInterval;
        }
        else if (debugMode == true)
        {
            Fire();

            schedulerIdleMotor = timestamp + IdleMotorWarningInterval;
        }
    }
}

void ValidateSpeed(void)
{
    speedDisplayed = (speedA * 100) + speedB;

    if (speedDisplayed < shootSpeedMinimum)
    {
        speedDisplayed = shootSpeedMinimum;
    }

    if (speedDisplayed > shootSpeedMaximum)
    {
        speedDisplayed = shootSpeedMaximum;
    }

    speedA = speedDisplayed / 100;
    speedB = speedDisplayed % 100;

    unsigned short speedDShotNew = speedDisplayed + dshotDelta;

    if (motorRunning == false)
    {
        speedDShot = speedDShotNew;
    }
    else
    {
        while (speedDShot < speedDShotNew)
        {
            speedDShot++;
            esc.setThrottle(speedDShot);
            delay(MotorSpeedUpDelay);
        }

        while (speedDShot > speedDShotNew)
        {
            speedDShot--;
            esc.setThrottle(speedDShot);
            delay(MotorSlowDownDelay);
        }
    }
}

bool IsLeftButtonPressed(void)
{
    return (digitalRead(PinButtonL) == LOW);
}

bool IsRightButtonPressed(void)
{
    return (digitalRead(PinButtonR) == LOW);
}

bool IsBallAvailable(void)
{
    Wire.requestFrom(I2C_ID_SERVO, 1);
    while (!Wire.available()) { }
    byte payload = Wire.read();
    return ((payload & BALL_STATE) == BALL_STATE);
}

void StartMotor(const bool permanent)
{
    PrintDebugLine("speed up");

    permanentMotorRun = permanent;

    motorRunning = true;

    for (unsigned short currentSpeed = 100;
         currentSpeed < speedDShot;
         currentSpeed++)
    {
        esc.setThrottle(currentSpeed);
        delay(MotorSpeedUpDelay);
    }

    ResetDebugLine();
}

void StopMotor(void)
{
    PrintDebugLine("slow down");

    for (unsigned short currentSpeed = speedDShot;
         currentSpeed > 100;
         currentSpeed--)
    {
        esc.setThrottle(currentSpeed);
        delay(MotorSlowDownDelay);
    }

    esc.setThrottle(0);

    motorRunning = false;

    permanentMotorRun = false;

    ResetDebugLine();
}

void StartServo(void)
{
    Wire.beginTransmission(I2C_ID_SERVO);
    Wire.write(byte(SERVO_START));
    Wire.endTransmission();
}

byte FetchServoStatus(void)
{
    Wire.requestFrom(I2C_ID_SERVO, 1);
    while (!Wire.available()) { }
    byte payload = Wire.read();
    return (payload & SERVO_MASK);
}

void Fire(void)
{
    bool shortShoot = (permanentMotorRun == false) ? true : false;

    if (shortShoot == true)
    {
        StartMotor(false);

        PrintDShotLine(speedDShot);

        PrintDebugLine("wait for RPM");
        delay(DelayAfterSpeedUp);
        ResetDebugLine();
    }

    do
    {
        PrintDebugLine("fire");

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
                        PrintDebugLine("ball load");
                        break;

                    case SERVO_TURN:
                        PrintDebugLine("ball turn");
                        break;

                    case SERVO_DONE:
                        ResetDebugLine();
                        break;
                }

                delay(100);
            }
        }
        while (servoStatus != SERVO_DONE);
    }
    while (IsRightButtonPressed() == true);

    if (shortShoot == true)
    {
        ResetDShotLine();

        StopMotor();
    }
}

void ProcessSetupMode(void)
{
    if ((IsLeftButtonPressed() == true) && (IsLeftButtonPressed() == true))
    {
        PrintSpecialLine("Entering setup mode");

        delay(EnteringSetupModeMessageDelay);

        PrintSpecialLine("Release buttons");

        while ((IsLeftButtonPressed() == true) || (IsLeftButtonPressed() == true)) { }

        PrintLeftButton("DEBUG");
        PrintRightButton("BATTERY");
        for (;;)
        {
            if (IsLeftButtonPressed() == true)
            {
                PrintSpecialLine("Running debug mode");

                debugMode = true;
                shootSpeedMinimum = shootSpeedMinimumDebugMode;
                shootSpeedMaximum = shootSpeedMaximumDebugMode;
                dshotDelta = DShotDeltaDebugMode;
                break;
            }
            if (IsRightButtonPressed() == true)
            {
                PrintSpecialLine("Battery level stored");

                batteryFullLevel = analogRead(A3);
                StoreToEEPROM(true);
                break;
            }
        }

        delay(LeavingSetupModeMessageDelay);

        while ((IsLeftButtonPressed() == true) || (IsLeftButtonPressed() == true)) { }
    }
}

void UpdateBatteryStatus(void)
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

    BatteryPrint(batteryLevelInPercent);
}

void IdleMotorWarning(void)
{
    PrintInfoLine2("STILL POWERED ON!?");
}

void LoadFromEEPROM(void)
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
        EEPROM.put(0x00, eepromData);
    }
}
