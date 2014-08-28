/**
 * LightwaveRF library for Raspberry Pi and Arduino. Utilising 433 MHz
 * receiver to interpret LightwaveRF remote controls.
 *
 * @author    James Reuss (jamesreuss.co.uk)
 * @copyright James Reuss 2014
 */

#include "LightwaveRX.h"

/*
 * Private variables
 */

static int  rxPin_ = 2;
static unsigned long lastInterruptTime_ = 0;
static uint8_t rxState_ = 0;
static const uint8_t rxState_idle = 0;
static const uint8_t rxState_msgstartfound = 1;
static const uint8_t rxState_bytestartfound = 2;
static const uint8_t rxState_getbyte = 3;
static uint8_t rxCurrentBits_ = 0; // number of bits in the current byte
static uint8_t rxNumBytes_ = 0; // number of bytes received
static bool messageComplete_ = false;
static const int messageLength_ = 10;
static uint8_t rawBuffer_[messageLength_];
static uint8_t rxBuffer_[messageLength_];
static uint8_t rxRepeats_ = 2;
static uint8_t rxRepeatCount_ = 0;
static uint8_t rxTimeout_ = 20; // reset repeat window after this in 100ms
static unsigned long lastPacketTime_ = 0; // ms
static const uint8_t nibbleBytes_[] = {0xF6,0xEE,0xED,0xEB,0xDE,0xDD,0xDB,0xBE,
                                        0xBD,0xBB,0xB7,0x7E,0x7D,0x7B,0x77,0x6F};

/*
 * Private functions
 */

void lwrx_process();
int16_t lwrx_findNibble(uint8_t data);

/*
 * Public function implementations
 */

void lwrx_setup(int rxPin, int interruptPin)
{
    rxPin_ = rxPin;

#ifdef ARDUINO
    attachInterrupt(interruptPin, lwrx_process, CHANGE);
#else
    wiringPiISR(interruptPin, INT_EDGE_BOTH, lwrx_process);
#endif
}

bool lwrx_gotMessage()
{
    return messageComplete_;
}

bool lwrx_getMessage(uint8_t *buffer, uint8_t length)
{
    bool ok = true;
    int16_t j = 0;

    if(messageComplete_ && length <= messageLength_)
    {
        for(uint8_t i = 0; ok && i < messageLength_; i++)
        {
            j = lwrx_findNibble(rawBuffer_[i]);
            if(j < 0) ok = false;

            switch(length) {
                case 4:
                    if(i == 9) buffer[2] =  j;
                    if(i == 2) buffer[3] =  j;
                case 2:
                    if(i == 3) buffer[0] =  j;
                    if(i == 0) buffer[1] =  j << 4;
                    if(i == 1) buffer[1] += j;
                    break;
                case 10:
                    buffer[i] = j;
                    break;
            }
        }

        messageComplete_ = false;
    }
    else
    {
        ok = false;
    }
    return ok;
}

/*
 * Private function implementations
 */

void lwrx_process()
{
    uint8_t event = digitalRead(rxPin_); // start setting event to the current value
    unsigned long curr = micros(); // the current time in microseconds

    uint16_t dur = (curr- lastInterruptTime_);
    lastInterruptTime_ = curr;
    //set event based on input and duration of previous pulse
    if(dur < 120) { //120 very short
    } else if(dur < 500) { // normal short pulse
      event +=2;
    } else if(dur < 2000) { // normal long pulse
      event +=4;
    } else if(dur > 5000){ // gap between messages
      event +=6;
    } else { //2000 > 5000
      event = 8; //illegal gap
    }

    //state machine transitions
    switch(rxState_) {
      case rxState_idle:
         switch(event) {
            case 7: //1 after a message gap
             rxState_ = rxState_msgstartfound;
             break;
         }
         break;
      case rxState_msgstartfound:
         switch(event) {
            case 2: //0 160->500
             //nothing to do wait for next positive edge
                break;
            case 3: //1 160->500
                rxNumBytes_ = 0;
                rxState_ = rxState_bytestartfound;
                break;
            default:
                //not good start again
                rxState_ = rxState_idle;
                break;
         }
         break;
      case rxState_bytestartfound:
         switch(event) {
            case 2: //0 160->500
                //nothing to do wait for next positive edge
                break;
            case 3: //1 160->500
                rxState_ = rxState_getbyte;
                rxCurrentBits_ = 0;
                break;
            case 5: //0 500->1500
                rxState_ = rxState_getbyte;
                // Starts with 0 so put this into byte
                rxCurrentBits_ = 1;
                rxBuffer_[rxNumBytes_] = 0;
                break;
            default:
                //not good start again
                rxState_ = rxState_idle;
                break;
         }
         break;
      case rxState_getbyte:
         switch(event) {
            case 2: //0 160->500
                //nothing to do wait for next positive edge but do stats
                /*if(lwrx_stats_enable) {
                  lwrx_stats[rx_stat_high_max] = max(lwrx_stats[rx_stat_high_max], dur);
                  lwrx_stats[rx_stat_high_min] = min(lwrx_stats[rx_stat_high_min], dur);
                  lwrx_stats[rx_stat_high_ave] = lwrx_stats[rx_stat_high_ave] - (lwrx_stats[rx_stat_high_ave] >> 4) + dur;
                }*/
                break;
            case 3: //1 160->500
                // a single 1
                rxBuffer_[rxNumBytes_] = rxBuffer_[rxNumBytes_] << 1 | 1;
                rxCurrentBits_++;
                /*if(lwrx_stats_enable) {
                  lwrx_stats[rx_stat_low1_max] = max(lwrx_stats[rx_stat_low1_max], dur);
                  lwrx_stats[rx_stat_low1_min] = min(lwrx_stats[rx_stat_low1_min], dur);
                  lwrx_stats[rx_stat_low1_ave] = lwrx_stats[rx_stat_low1_ave] - (lwrx_stats[rx_stat_low1_ave] >> 4) + dur;
                }*/
                break;
            case 5: //1 500->1500
                // a 1 followed by a 0
                rxBuffer_[rxNumBytes_] = rxBuffer_[rxNumBytes_] << 2 | 2;
                rxCurrentBits_++;
                rxCurrentBits_++;
                /*if(lwrx_stats_enable) {
                  lwrx_stats[rx_stat_low0_max] = max(lwrx_stats[rx_stat_low0_max], dur);
                  lwrx_stats[rx_stat_low0_min] = min(lwrx_stats[rx_stat_low0_min], dur);
                  lwrx_stats[rx_stat_low0_ave] = lwrx_stats[rx_stat_low0_ave] - (lwrx_stats[rx_stat_low0_ave] >> 4) + dur;
                }*/
                break;
            default:
                //not good start again
                rxState_ = rxState_idle;
                break;
         }
         if(rxCurrentBits_ >= 8) {
            rxNumBytes_++;
            rxCurrentBits_ = 0;
            if(rxNumBytes_ >= messageLength_) {
                unsigned long currMillis = millis();
                if(rxRepeats_ > 0) {
                  if((currMillis - lastPacketTime_) / 100 > rxTimeout_) {
                     rxRepeatCount_ = 1;
                  } else {
                     //Test message same as last one
                     int16_t i = messageLength_; //int
                     do {
                        i--;
                     }
                     while((i >= 0) && (rawBuffer_[i] == rxBuffer_[i]));
                     if(i < 0) {
                        rxRepeatCount_++;
                     } else {
                        rxRepeatCount_ = 1;
                     }
                  }
                } else {
                  rxRepeatCount_ = 0;
                }
                lastPacketTime_ = currMillis;
                //If last message hasn't been read it gets overwritten
                memcpy(rawBuffer_, rxBuffer_, messageLength_);
                // And cycle round for next one
                rxState_ = rxState_idle;
            } else {
              rxState_ = rxState_bytestartfound;
            }
         }
         break;
    }
}

int16_t lwrx_findNibble(uint8_t data)
{
    int16_t i = 15;
    do {
      if(nibbleBytes_[i] == data) break;
      i--;
    } while (i >= 0);
    return i;
}
