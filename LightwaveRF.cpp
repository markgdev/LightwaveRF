// LightwaveRF.cpp
//
// LightwaveRF 434MHz interface for Arduino
// 
// Author: Lawrie Griffiths (lawrie.griffiths@ntlworld.com)
// Copyright (C) 2013 Lawrie Griffiths

#include "LightwaveRF.h"

#ifndef ARDUINO
#include <string.h>
#endif

static const uint8_t nibbleBytes_[] = {0xF6,0xEE,0xED,0xEB,0xDE,0xDD,0xDB,0xBE,0xBD,0xBB,0xB7,0x7E,0x7D,0x7B,0x77,0x6F};

/*
 * Public functions
 */

bool lightwaverf_setup()
{
    return lightwaverf_setup(3, 2, 2);
}

void lightwaverf_setup(int txPin)
{
    lightwaverf_txPin_ = txPin;
    pinMode(lightwaverf_txPin_, OUTPUT);
}

void lightwaverf_setup(int rxPin, int interruptPin)
{
    lightwaverf_rxPin_ = rxPin;
    lightwaverf_interruptPin_ = interruptPin;
    pinMode(lightwaverf_rxPin_, INPUT);
#ifdef ARDUINO
    attachInterrupt(lightwaverf_interruptPin_, lightwaverf_processBits, CHANGE);
#else
    wiringPiISR(lightwaverf_interruptPin_, INT_EDGE_BOTH, lightwaverf_processBits);
#endif
}

bool lightwaverf_setup(int txPin, int rxPin, int interruptPin)
{
    if (txPin == rxPin)
        return false;

    lightwaverf_setup(txPin);
    lightwaverf_setup(rxPin, interruptPin);
    return true;
}

void lightwaverf_sendMsg(uint8_t *msg)
{
#ifdef ARDUINO
    cli();
#endif
    for (uint8_t j = lightwaverf_repeatCount_; j > 0; j--)
    {
        // Send the start bit
        lightwaverf_sendBit(HIGH);
        
        // Send the 10 bytes of the msg
        for (uint8_t i = 0; i < lightwaverf_messageLength_; i++)
        {
            lightwaverf_sendByte(msg[i]);
        }
        
        // Send the end bit
        lightwaverf_sendBit(HIGH);
    
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

void lightwaverf_sendCmd(uint8_t level, uint8_t channel, uint8_t cmd, uint8_t *id)
{
    uint8_t msg[10];
    
    msg[0] = nibbleBytes_[level >> 4];
    msg[1] = nibbleBytes_[level & 0xF];
    msg[2] = nibbleBytes_[channel];
    msg[3] = nibbleBytes_[cmd];
    
    for(uint8_t i = 0; i < 6; i++)
    {
        msg[4 + i] = id[i];
    }
    lightwaverf_sendMsg(msg);
}

void lightwaverf_getErrorStats(long *inv)
{
    for(int i = 0; i < 4; i++)
    {
        inv[i] = lightwaverf_invalidPacketsCount_[i];
    }
}

bool lightwaverf_gotMessage()
{
    return lightwaverf_gotMessage_;
}

bool lightwaverf_getMessage(uint8_t *buffer, uint8_t *length)
{
    if (!lightwaverf_gotMessage_) return false;
    
    // Buffer length must be 10
    if (*length != lightwaverf_messageLength_) return false;
    
    memcpy(buffer, lightwaverf_buffer_, lightwaverf_messageLength_);

    lightwaverf_gotMessage_ = false;
    return true;
}

/*
 * Private functions
 */

void lightwaverf_sendBit(uint8_t b)
{
    delayMicroseconds(25);
    digitalWrite(lightwaverf_txPin_, b);
    delayMicroseconds(lightwaverf_bitDelay_/2);
    digitalWrite(lightwaverf_txPin_, LOW);
    delayMicroseconds(lightwaverf_bitDelay_/2);

    if (b == LOW) {
        delayMicroseconds(300);
    }
}

void lightwaverf_sendByte(uint8_t b)
{
    lightwaverf_sendBit(HIGH);

    for (uint8_t mask = 0b10000000; mask; mask >>= 1)
    {
        lightwaverf_sendBit(b & mask);
    }
}

void lightwaverf_processBits()
{ 
    // Don't process bits when a message is already ready
    if (lightwaverf_gotMessage_) return;
    
    uint8_t v = digitalRead(lightwaverf_rxPin_); // the current value
    unsigned long curr = micros(); // the current time in microseconds
    
    // Calculate pulse duration in 50 microsecond units
    unsigned int dur = (curr - lightwaverf_previousTime_)/50;
    lightwaverf_previousTime_ = curr;
    
    // See if pulse corresponds to expected bit length
    if (dur < 6) {
        // inter 1 bit gap - do nothing
    } else if (dur < 11) {
        // potential 1 bit
        if (!v) { // value now zero as 1 pulse ended
            // 1 bit
            if (!lightwaverf_packetStarted_) {
                // Start of message
                lightwaverf_packetStarted_ = true;
                lightwaverf_byteStarted_ = false;
                lightwaverf_bytesReceived_ = 0;
            } else if (!lightwaverf_byteStarted_) {
                // byte started
                lightwaverf_byteStarted_ = true;
                lightwaverf_bitsCount_ = 0;
                lightwaverf_byte_ = 0;
            } else {
                // a valid 1 bit
                lightwaverf_byte_ = (lightwaverf_byte_ << 1) | 1;
                if (++lightwaverf_bitsCount_ == 8) { // Test for complete byte
                    // Add the byte to the message
                    lightwaverf_byteStarted_ = false;
                    lightwaverf_buffer_[lightwaverf_bytesReceived_++] = lightwaverf_byte_;
                }
            }
        } else {
            // Too short for a zero bit
            lightwaverf_packetStarted_ = false;
            if (lightwaverf_bytesReceived_ > 0) lightwaverf_invalidPacketsCount_[0]++;
        }
    } else if (dur > 20 && dur < 28) {
        // potential 0 bit
        if (v) {
            // 0 bit
            if (!lightwaverf_byteStarted_) {
                // Zero bit where byte start bit expected
                lightwaverf_packetStarted_ = false;
        if (lightwaverf_bytesReceived_ > 0) lightwaverf_invalidPacketsCount_[1]++;
            } else if (lightwaverf_packetStarted_) {
                // Valid 0 bit
                lightwaverf_byte_ = (lightwaverf_byte_ << 1);
                if (++lightwaverf_bitsCount_ == 8) {
                    // Add the byte to the message
                    lightwaverf_buffer_[lightwaverf_bytesReceived_++] = lightwaverf_byte_;
                    lightwaverf_byteStarted_ = false;
                }
            }
        } else {
            // Too long for a 1 bit
            lightwaverf_packetStarted_ = false;
            if (lightwaverf_bytesReceived_ > 0) lightwaverf_invalidPacketsCount_[2]++;
        }
    } else {
        // Not a valid length for a bit
        if (lightwaverf_packetStarted_ && lightwaverf_bytesReceived_ > 0) lightwaverf_invalidPacketsCount_[3]++;
        lightwaverf_packetStarted_ = false;
    }
    
    // See if we have the whole message
    if ((lightwaverf_packetStarted_ && lightwaverf_bytesReceived_) == lightwaverf_messageLength_) {
        lightwaverf_gotMessage_ = true;
        lightwaverf_packetStarted_ = false;
    }
}
