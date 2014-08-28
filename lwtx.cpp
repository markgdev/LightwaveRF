#include <stdio.h>
#include <wiringPi.h>
#include "LightwaveTX.h"

void loop();
void turnOn();
void turnDim();
void turnOff();

uint8_t on[] = {0xf6,0xf6,0xf6,0xee,0x6f,0xeb,0xbe,0xed,0xb7,0x7b};
uint8_t off[] = {0xf6,0xf6,0xf6,0xf6,0x6f,0xeb,0xbe,0xed,0xb7,0x7b};
uint8_t id[] = {0x6F, 0xEB, 0xBE, 0xED, 0xB7, 0x7B};
int time = 0;

int main(int argc, char const *argv[])
{
    printf("lwtx starting\n");
    wiringPiSetup();
    lwtx_setup(3);

    while (1) {
        loop();
    }

    return 0;
}

void loop() {

    printf("time: %d", (time * 5));

    if (time % 3 == 0) turnOn();
    else if (time % 2 == 0) turnDim();
    else turnOff();

    time++;

    delay(5000); // 5 secs
}

void turnOn() {
    printf("\tturning on\n");
    lwtx_sendCmd(0x80 + 31,     // level
                 6,             // byte channel
                 Lightwave_On,  // byte command
                 id);           // byte* id
}

void turnDim() {
    printf("\tturning dim\n");
    lwtx_sendCmdDim(12,     // dim value
                    6,      // byte channel
                    id);    // byte* id
}

void turnOff() {
    printf("\tturning off\n");
    lwtx_sendMsg(off);
}
