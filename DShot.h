#pragma once

#include "Arduino.h"

class DShot
{
public:
    DShot();
    void attach(int pin);
    void setThrottle(unsigned int throttle);

private:
    uint8_t pinMask = 0;
};
