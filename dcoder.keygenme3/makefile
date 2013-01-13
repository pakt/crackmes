all: sum4.o siphash.o
	gcc -o keygen sum4.o siphash.o

sum4.o: sum4.c
	gcc -c -O3 -Wall sum4.c

siphash.o: siphash.c
	gcc -c -O3 -std=c99 siphash.c

clean: 
	rm *.o keygen
