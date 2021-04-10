#pragma once

#define LCD

void InitDisplay(void);

void ResetDisplay(void);

void PrintBatteryStatus(const byte batteryLevel);

void PrintDebugLine(const String message);

void DisplaySpeed(const unsigned short speedA, const unsigned short speedB, const bool debugMode);

void PrintDShotLine(const unsigned short speedDShot);

void ResetDShotLine(void);

void PrintBallInfoLine(const String message);

void ResetBallInfoLine(void);

void PrintStatusLine(const String message);

void ResetStatusLine(void);

void PrintLeftButton(const String buttonLabel);

void PrintRightButton(const String buttonLabel);
