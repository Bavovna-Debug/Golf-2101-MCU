#pragma once

const int I2C_ID_SERVO  = 0x08;
const int I2C_ID_WIFI   = 0x07;

const byte BALL_MASK    = 0xF0;
const byte SERVO_MASK   = 0x0F;

const byte BALL_STATE   = 0x80;

const byte READ_DIP     = 0x01;
const byte SERVO_START  = 0x08;
const byte SERVO_LOAD   = 0x09;
const byte SERVO_WAIT   = 0x0A;
const byte SERVO_TURN   = 0x0B;
const byte SERVO_DONE   = 0x0F;

struct WiFiRequest
{
    uint16_t speedA;
    uint16_t speedB;
};

struct WiFiResponse
{
    int16_t speedADelta;
    int16_t speedBDelta;
};
