all: kg.c smth.o
	gcc -g -o kg -Iskein/ skein/*.o smth.o kg.c -lgmp -lm

smth.o: smth.asm
	nasm -felf smth.asm -o smth.o

clean:
	rm kg *.o
