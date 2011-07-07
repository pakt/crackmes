#
ODIR = mingw-obj
#
_OBJS = big.o ecn.o miracl.a
#
OBJS = $(patsubst %,$(ODIR)\\%,$(_OBJS))
#
LIBS = -lhid -lsetupapi
#
FLAGS = -Wall -Ic:\miracl\include -I..\usb_rawhid -I..\rawhid_test -static

all: ecnr.o keygen.o crc32.o hid.o
	g++  $(OBJS) ..\rawhid_test\hid.o ecnr.o keygen.o crc32.o $(LIBS) -o kg.exe

hid.o:
	cd "..\rawhid_test";$(MAKE)

ecnr.o: ecnr.cpp ecnr.h
	g++ -c $(FLAGS)  ecnr.cpp

crc32.o: crc32.cpp crc32.h
	g++ -c crc32.cpp

keygen.o: keygen.cpp
	g++ -c $(FLAGS) keygen.cpp

clean:
	rm *.o
	cd "..\rawhid_test";rm *.o
