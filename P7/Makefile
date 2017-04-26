CC=gcc
CFLAGS=-g -Wall

all:	TTT ttt

TTT:	TTT.o msg.o
	gcc -o TTT TTT.o msg.c

ttt:	ttt.o msg.o
	gcc -o ttt ttt.o child.c msg.c


TTT.o:	TTT.c common.h

ttt.o:	ttt.c common.h

clean:
	rm -f *.o ttt TTT serverloc
