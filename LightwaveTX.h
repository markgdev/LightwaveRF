/**
 * LightwaveRF library for Raspberry Pi and Arduino. Utilising 433 MHz
 * transmitter to control LightwaveRF devices.
 *
 * @author    James Reuss (jamesreuss.co.uk)
 * @copyright James Reuss 2014
 */

#ifndef LIGHTWAVETX_H
#define LIGHTWAVETX_H

#ifdef ARDUINO
    #include <Arduino.h>
    #define uint8_t byte
    #define bool boolean
#else
    #include <wiringPi.h>
    #include <stdint.h>
#endif

const uint8_t Lightwave_LevelOffset = 0x80;
const uint8_t Lightwave_LevelMax = Lightwave_LevelOffset + 31;
const uint8_t Lightwave_LevelOff = 0x40;

enum Lightwave_Cmd
{
    Lightwave_Off = 0,
    Lightwave_On = 1,
    Lightwave_Mood = 2,
    Lightwave_Lock = 4
};

/**
 * Setup the system and TX pin
 * @param  txPin The pin to use for TX data
 */
void lwtx_setup(int txPin);

/**
 * Send a complete message buffer
 * @param msg A 10 byte message buffer to send
 */
void lwtx_sendMsg(uint8_t *msg);

/**
 * Build and send a structured LightwaveRF message
 * @param level   The dim level command for the light
 * @param channel The channel to transmit on
 * @param cmd     The command to send (see Lightwave_Cmd)
 * @param id      The destination device ID (6 bytes)
 */
void lwtx_sendCmd(uint8_t level, uint8_t channel, Lightwave_Cmd cmd, uint8_t *id);

/**
 * Helper function to send the command to turn a light ON to its highest
 * brightness.
 * @param channel The channel to transmit on
 * @param id      The destination device ID (6 bytes)
 */
void lwtx_sendCmdOnMax(uint8_t channel, uint8_t *id);

/**
 * Helper function to send the command to turn a light ON and dim to a certain
 * level (1-31).
 * @param dim     The dim level to set the light to (1-31)
 * @param channel The channel to transmit on
 * @param id      The destination device ID (6 bytes)
 */
void lwtx_sendCmdDim(uint8_t dim, uint8_t channel, uint8_t *id);

/**
 * Helper function to send the command to turn a light OFF.
 * @param channel The channel to transmit on
 * @param id      The destination device ID (6 bytes)
 */
void lwtx_sendCmdOff(uint8_t channel, uint8_t *id);

#endif // LIGHTWAVETX_H
