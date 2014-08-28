#include <stdio.h>
#include <wiringPi.h>
#include "LightwaveRX.h"

void loop();
void printMsg(uint8_t *msg, uint8_t len);

int main(int argc, char const *argv[])
{
    printf("lwrx starting\n");
    wiringPiSetup();
    lwrx_setup(2, 2);

    while (1) {
        loop();
    }

    return 0;
}

void loop() {
    uint8_t msg[10];
    uint8_t len = 10;

    if (lwrx_gotMessage())
    {
        bool ok = lwrx_getMessage(msg, len);
        if (ok) {
            printMsg(msg, len);
        } else {
            printf("failed to get message\n");
        }
    }

    delay(20);
}

void printMsg(uint8_t *msg, uint8_t len) {
    printf("msg recv'd: ");
    for(int i=0; i<len; i++) {
        printf("0x%02x ", msg[i]);
    }
    printf("\n");
}
