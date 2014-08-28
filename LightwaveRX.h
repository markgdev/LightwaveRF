/**
 * LightwaveRF library for Raspberry Pi and Arduino. Utilising 433 MHz
 * receiver to interpret LightwaveRF remote controls.
 *
 * @author    James Reuss (jamesreuss.co.uk)
 * @copyright James Reuss 2014
 */

#ifndef LIGHTWAVERX_H
#define LIGHTWAVERX_H

#ifdef ARDUINO
    #include <Arduino.h>
    #define uint8_t byte
    #define bool boolean
#else
    #include <wiringPi.h>
    #include <stdint.h>
 	#include <string.h>
#endif

/**
 * Setup RX pins and interrupts
 * @param  rxPin     The pin to use for RX data
 * @param  interrupt The pin to use for RX interrupt
 */
void lwrx_setup(int rxPin, int interruptPin);

bool lwrx_gotMessage();

bool lwrx_getMessage(uint8_t *buffer, uint8_t length);

#endif // LIGHTWAVERX_H