/**
 * LightwaveRF library for Raspberry Pi and Arduino. Utilising 433 MHz
 * transmitter to control LightwaveRF devices.
 *
 * @author    James Reuss (jamesreuss.co.uk)
 * @copyright James Reuss 2014
 */

#include "LightwaveTX.h"

/*
 * Private variables
 */

static int txPin_ = 3;
static const int messageLength_ = 10;
static const int bitDelay_      = 560;
static const int repeatCount_   = 12;
static const uint8_t nibbleBytes_[] = {0xF6,0xEE,0xED,0xEB,0xDE,0xDD,0xDB,0xBE,
                                       0xBD,0xBB,0xB7,0x7E,0x7D,0x7B,0x77,0x6F};

/*
 * Private functions
 */

static void lwtx_sendBit(uint8_t b); // implement
static void lwtx_sendByte(uint8_t b); // implement

/*
 * Public function implementations
 */

void lwtx_setup(int txPin)
{
    txPin_ = txPin;
    pinMode(txPin_, OUTPUT);
}

void lwtx_sendMsg(uint8_t *msg)
{
#ifdef ARDUINO
    cli();
#endif
    for (uint8_t j = repeatCount_; j > 0; j--)
    {
        // Send the start bit
        lwtx_sendBit(HIGH);

        // Send the 10 bytes of the msg
        for (uint8_t i = 0; i < messageLength_; i++)
        {
            lwtx_sendByte(msg[i]);
        }

        // Send the end bit
        lwtx_sendBit(HIGH);

        // Delay between repeats
        if (j > 1)
        {
            delayMicroseconds(10250);
        }
    }
#ifdef ARDUINO
    sei();
#endif
}

void lwtx_sendCmd(uint8_t level, uint8_t channel, Lightwave_Cmd cmd, uint8_t *id)
{
    uint8_t msg[messageLength_];

    msg[0] = nibbleBytes_[level >> 4];
    msg[1] = nibbleBytes_[level & 0xF];
    msg[2] = nibbleBytes_[channel];
    msg[3] = nibbleBytes_[cmd];

    for(uint8_t i = 0; i < 6; i++)
    {
        msg[4 + i] = id[i];
    }
    lwtx_sendMsg(msg);
}

void lwtx_sendCmdOnMax(uint8_t channel, uint8_t *id)
{
    lwtx_sendCmd(Lightwave_LevelMax, channel, Lightwave_On, id);
}

void lwtx_sendCmdDim(uint8_t dim, uint8_t channel, uint8_t *id)
{
    dim = (dim <= 31) ? dim : 31;
    lwtx_sendCmd(Lightwave_LevelOffset + dim, channel, Lightwave_On, id);
}

void lwtx_sendCmdOff(uint8_t channel, uint8_t *id)
{
    lwtx_sendCmd(Lightwave_LevelOff, channel, Lightwave_Off, id);
}

/*
 * Private function implementations
 */

void lwtx_sendBit(uint8_t bit)
{
    delayMicroseconds(25);
    digitalWrite(txPin_, bit);
    delayMicroseconds(bitDelay_/2);
    digitalWrite(txPin_, LOW);
    delayMicroseconds(bitDelay_/2);

    if (bit == LOW) {
        delayMicroseconds(300);
    }
}

void lwtx_sendByte(uint8_t byte)
{
    lwtx_sendBit(HIGH);

    for (uint8_t mask = 0b10000000; mask; mask >>= 1)
    {
        lwtx_sendBit(byte & mask);
    }
}

