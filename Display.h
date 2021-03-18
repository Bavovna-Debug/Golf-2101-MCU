#pragma once

#define LCD

void InitDisplay();

void ResetDisplay();

void PrintBatteryStatus(const byte batteryLevel);

void PrintDebugLine(const String message);

void DisplaySpeed(const unsigned short speedA, const unsigned short speedB);

void PrintDShotLine(const unsigned short speedDShot);

void ResetDShotLine();

void PrintBallInfoLine(const String message);

void ResetBallInfoLine();

void PrintStatusLine(const String message);

void ResetStatusLine();

void PrintLeftButton(const String buttonLabel);

void PrintRightButton(const String buttonLabel);
