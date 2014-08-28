DEBUG	= -O3
CC		= gcc
INCLUDE	= -I/usr/local/include -I./
CFLAGS	= $(DEBUG) -Wall $(INCLUDE) -Winline -pipe

LDFLAGS	= -L/usr/local/lib
LDLIBS  = -lwiringPi

.PHONY: all clean

all: lwtx lwrx

clean:
	-rm -f *.o

lwrx: lwrx.o LightwaveRX.o
	$(CC) -o lwrx lwrx.o LightwaveRX.o $(LDFLAGS) $(LDLIBS)

lwtx: lwtx.o LightwaveTX.o
	$(CC) -o lwtx lwtx.o LightwaveTX.o $(LDFLAGS) $(LDLIBS)

lwrx.o: LightwaveRX.h
	$(CC) $(CFLAGS) -c lwrx.cpp

lwtx.o: LightwaveTX.h
	$(CC) $(CFLAGS) -c lwtx.cpp

LightwaveRX.o: LightwaveRX.cpp LightwaveRX.h
	$(CC) $(CFLAGS) -c LightwaveRX.cpp

LightwaveTX.o: LightwaveTX.cpp LightwaveTX.h
	$(CC) $(CFLAGS) -c LightwaveTX.cpp
