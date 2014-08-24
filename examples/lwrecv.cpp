#include <stdio.h>
#include <wiringPi.h>
#include "LightwaveRF.h"

void loop();
void printMsg(uint8_t *msg, uint8_t len);

int main(void) {
    printf("lwrecv starting\n");
    wiringPiSetup();
    lightwaverf_setup(2, 2);
    //lw_rx_setup(2, 0);

    while (1) {
        loop();
    }
}

void loop() {
    uint8_t msg[10];
    uint8_t len = 10;

    //if (lw_have_message())
    if (lightwaverf_gotMessage())
    {
        //lw_get_message(msg, &len);
        bool ok = lightwaverf_getMessage(msg, &len);
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
