CC = gcc -I/usr/include/libusb-1.0
GXX = g++
CXXFLAGS = -pipe -O2 -Wall -W -D_REENTRANT -lusb-1.0 -lm
INCPATH = -I. -I/usr/include/libusb-1.0

all: flirone.o flirone flir

flirone.o: src/flirone.c src/plank.h

flirone: src/flirone.o
	${CC} -o bin/flirone src/flirone.o -lusb-1.0 -lm -Wall

flir:
	${CC} -o bin/flir src/flir8g-mint.c -lusb-1.0 -lm

clean:
	rm -f src/flirone.o bin/flirone bin/flir