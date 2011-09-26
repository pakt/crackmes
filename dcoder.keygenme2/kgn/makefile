
all: lambda.c keygen.c hash.o
	gcc -Wall -O3 -o lambda lambda.c  -lflint -lmpir -lmpfr 
	gcc -Wall -o keygen keygen.c hash.o -lflint -lmpir -lmpfr 

hash.o: hash.asm
	nasm -f elf hash.asm -o hash.o

clean:
	rm *.o keygen lambda
