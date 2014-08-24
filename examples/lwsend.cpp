#include <stdio.h>
#include <wiringPi.h>
#include "LightwaveRF.h"

void loop();
void turnOn();
void turnDim();
void turnOff();

uint8_t on[] = {0xf6,0xf6,0xf6,0xee,0x6f,0xeb,0xbe,0xed,0xb7,0x7b};
uint8_t off[] = {0xf6,0xf6,0xf6,0xf6,0x6f,0xeb,0xbe,0xed,0xb7,0x7b};
uint8_t id[] = {0x6F, 0xEB, 0xBE, 0xED, 0xB7, 0x7B};
int time = 0;

int main(void) {
    printf("lwsend starting\n");
    wiringPiSetup();

    //lw_setup();
    //lw_tx_setup(3);
    lightwaverf_setup(3);

    while (1) {
        loop();
    }
}

void loop() {

    printf("time = %d, turning %s\n", (time * 5), ((time % 2 == 0) ? "on" : "off"));

    if (time % 3 == 0) turnOn();
    else if (time % 2 == 0) turnDim();
    else turnOff();

    time++;

    delay(5000); // 5 secs
}

void turnOn() {
    printf("\tTurning on\n");
    //lw_send(on);
    //lw_cmd(0x80 + 31,
    lightwaverf_sendCmd(0x80 + 31,
        6,       // byte channel
        Lightwave_On,   // byte command
        id); // byte* id
}

void turnDim() {
    printf("\tTurning dim\n");
    //lw_cmd(0x80 + 12,
    lightwaverf_sendCmd(0x80 + 12,
        6,       // byte channel
        Lightwave_On,   // byte command
        id); // byte* id
}

void turnOff() {
    printf("\tTurning off\n");
    //lw_send(off);
    lightwaverf_sendMsg(off);
}
