/*
 * LightwaveRF 433 MHz library for the Raspberry Pi and Arduino.
 * @copyright James Reuss 2014 (jamesreuss.co.uk)
 */

#ifdef ARDUINO
    #include <Arduino.h>
    #define uint8_t byte
    #define bool boolean
#else
    #include <wiringPi.h>
    #include <stdint.h>
#endif

enum Lightwave_State
{
    Lightwave_Off = 0,
    Lightwave_On = 1,
    Lightwave_Mood = 2,
    Lightwave_Lock = 4
};

/**
 * Setup TX, RX pins and interrupts using defaults
 * @return False if there was an error during setup
 */
bool lightwaverf_setup();
/**
 * Setup TX pin
 * @param  txPin The pin to use for TX data
 */
void lightwaverf_setup(int txPin);
/**
 * Setup RX pins and interrupts
 * @param  rxPin     The pin to use for RX data
 * @param  interrupt The pin to use for RX interrupt
 */
void lightwaverf_setup(int rxPin, int interruptPin);
/**
 * Setup TX and RX pins and interrupts
 * @param  txPin        The pin to use for TX data
 * @param  rxPin        The pin to use for RX data
 * @param  interruptPin The pin to use for RX interrupt
 * @return              False if there was an error during setup
 */
bool lightwaverf_setup(int txPin, int rxPin, int interruptPin);

/**
 * Send a complete message buffer
 * @param msg A 10 byte message buffer to send
 */
void lightwaverf_sendMsg(uint8_t *msg);
/**
 * Build and send a structured LightwaveRF message
 * @param level   The dim level command for the light
 * @param channel The channel
 * @param cmd     [description]
 * @param id      [description]
 */
void lightwaverf_sendCmd(uint8_t level, uint8_t channel, uint8_t cmd, uint8_t *id);

void lightwaverf_getErrorStats(long *inv);

bool lightwaverf_gotMessage();
bool lightwaverf_getMessage(uint8_t *buffer, uint8_t *length);


static int lightwaverf_txPin_;
static int lightwaverf_rxPin_;
static int lightwaverf_interruptPin_;

static volatile bool lightwaverf_gotMessage_;
static const int lightwaverf_messageLength_ = 10;  // expected length of message
static uint8_t lightwaverf_buffer_[lightwaverf_messageLength_]; // received message
static uint8_t lightwaverf_byte_;      // current byte
static uint8_t lightwaverf_bitsCount_; // number of bits in current byte
static unsigned long lightwaverf_previousTime_; // time of previous pulse
static volatile bool lightwaverf_packetStarted_;
static bool lightwaverf_byteStarted_;
static uint8_t lightwaverf_bytesReceived_;

static const int lightwaverf_bitDelay_ = 560;
static const int lightwaverf_repeatCount_ = 12; // how many times to repeat outgoing messages

static long lightwaverf_invalidPacketsCount_[4];

/**
 * Transmit a single bit over RF 433
 * @param b The bit (0, 1) to send
 */
static void lightwaverf_sendBit(uint8_t b);
/**
 * Send a byte over RF 433
 * @param b The byte to send
 */
static void lightwaverf_sendByte(uint8_t b);

/**
 * ISR called when 433 MHz receiver data pin changes state.
 * The ISR identifies the received LightwaveRF bits and reconstructs the
 * message if it is valid.
 */
static void lightwaverf_processBits();
