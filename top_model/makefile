CC=g++
CFLAGS=-std=c++17
COMPILE_TARGET=DISCO_F429ZI
EXECUTABLE_NAME=BLINKY_TOP

INCLUDECADMIUM=-I ../../cadmium/include
INCLUDEDESTIMES=-I ../../cadmium/DESTimes/include

embedded:
	mbed compile --target $(COMPILE_TARGET) --toolchain GCC_ARM --profile ../cadmium.json

flash:
	st-flash write ../BUILD/$(COMPILE_TARGET)/GCC_ARM-CADMIUM/*.bin 0x8000000
	$(info *** FLASH WILL TAKE ~15 Seconds! DO NOT RESET WHILE COM PORT LED IS FLASHING! ***)

all: main.o ../data_structures/message.o
	$(CC) -g -o $(EXECUTABLE_NAME) main.o ../data_structures/message.o

main.o: main.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) main.cpp -o main.o

../data_structures/message.o:
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) ../data_structures/message.cpp -o ../data_structures/message.o

clean:
	rm -f $(EXECUTABLE_NAME) *.o *~
	-for d in ../data_structures; do (cd $$d; rm -f *.o *~ ); done

eclean:
	rm -rf ../BUILD