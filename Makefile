DEBUG	= -O3
CC		= gcc
INCLUDE	= -I/usr/local/include -I./
CFLAGS	= $(DEBUG) -Wall $(INCLUDE) -Winline -pipe

LDFLAGS	= -L/usr/local/lib
LDLIBS  = -lwiringPi

.PHONY: all clean

all: lwsend lwrecv

lwrecv: lwrecv.o LightwaveRF.o
	$(CC) -o lwrecv lwrecv.o LightwaveRF.o $(LDFLAGS) $(LDLIBS)

lwsend: lwsend.o LightwaveRF.o
	$(CC) -o lwsend lwsend.o LightwaveRF.o $(LDFLAGS) $(LDLIBS)

lwrecv.o: LightwaveRF.h
	$(CC) $(CFLAGS) -c ./examples/lwrecv.cpp

lwsend.o: LightwaveRF.h
	$(CC) $(CFLAGS) -c ./examples/lwsend.cpp

LightwaveRF.o: LightwaveRF.h
	$(CC) $(CFLAGS) -c LightwaveRF.cpp

clean:
	-rm -f *.o
