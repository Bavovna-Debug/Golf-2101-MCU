#include "Arduino.h"
#include "DShot.h"

static uint8_t dShotBits[16];

static uint8_t dShotPins = 0;

ISR(TIMER1_COMPA_vect)
{
    noInterrupts();

    asm(
        "LDI r23, 0 \n"
        "IN r25, %0 \n"
        "_for_loop: \n"
        "OR r25, %1 \n"
        "OUT %0, r25 \n"
        "NOP \n"
        "NOP \n"
        "NOP \n"
        "NOP \n"
        "NOP \n"
        "NOP \n"
        "LD r24, Z+ \n"
        "AND r25, r24 \n"
        "OUT %0, r25 \n"
        "NOP \n"
        "NOP \n"
        "NOP \n"
        "NOP \n"
        "NOP \n"
        "NOP \n"
        "NOP \n"
        "NOP \n"
        "AND r25, %2 \n"
        "OUT %0, r25 \n"
        "INC r23 \n"
        "CPI r23, 16 \n"
        "BRLO _for_loop \n"
        :
        : "I" (_SFR_IO_ADDR(PORTB)), "r" (dShotPins), "r" (~dShotPins), "z" (dShotBits)
        : "r25", "r24", "r23"
    );

    interrupts();
}

// Prepare data packet, attach 0 to telemetry bit, and calculate CRC.
// Throttle is given as 11-bit value.
//
static inline uint16_t DShotPacket(uint16_t throttle)
{
    uint8_t checksum = 0;

    throttle <<= 1;

    if (throttle < 48)
    {
        throttle |= 1;
    }

    uint16_t checksumData = throttle;
    for (byte i = 0; i < 3; i++)
    {
        checksum ^= checksumData;
        checksumData >>= 4;
    }
    checksum &= 0xf;

    return ((throttle << 4) | checksum);
}

DShot::DShot()
{ }

void DShot::attach(int pin)
{
    this->pinMask = digitalPinToBitMask(pin);

    pinMode(pin, OUTPUT);

    for (byte i = 0; i < 16; i++)
    {
        dShotBits[i] = 0;
        dShotPins = 0;
    }

    dShotPins |= this->pinMask;

    cli();

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // Set compare match register for 500 Hz increments.
    //
    OCR1A = 31999; // 16.000.000 / (1 * 500) - 1

    // Turn on CTC mode.
    //
    TCCR1B |= (1 << WGM12);

    // Set CS12, CS11 and CS10 bits for 1 prescaler.
    //
    TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);

    // Enable timer compare interrupt.
    //
    TIMSK1 |= (1 << OCIE1A);

    sei();
}

/*
  Set the throttle value and prepare the data packet and store
  throttle: 11-bit data
*/
void DShot::setThrottle(unsigned int throttle)
{
    uint16_t packet = DShotPacket(throttle);
    uint16_t mask = 0x8000;

    for (byte i = 0; i < 16; i++)
    {
        if (packet & mask)
        {
            dShotBits[i] |= this->pinMask;
        }
        else
        {
            dShotBits[i] &= ~(this->pinMask);
        }

        mask >>= 1;
    }
}
