#pragma once

#define LCD

void InitDisplay(void);

void ResetDisplay(void);

void DisplaySpeed(const unsigned short speedA, const unsigned short speedB, const bool debugMode);

void PrintDShotLine(const unsigned short speedDShot);
void ResetDShotLine(void);

void PrintInfoLine1(const String message);
void ResetInfoLine1(void);

void PrintInfoLine2(const String message);
void ResetInfoLine2(void);

void PrintDebugLine(const String message);
void ResetDebugLine(void);

void PrintSpecialLine(const String message);
void PrintLeftButton(const String buttonLabel);
void PrintRightButton(const String buttonLabel);

void BatteryInit(void);
void BatteryPrint(const byte level);
