CC=gcc
CFLAGS=-g -Wall

all:	TTTS ttt

TTT:	TTTS.o msg.o
	gcc -o TTT TTTS.o msg.c

ttt:	ttt.o msg.o
	gcc -o ttt ttt.o child.c msg.c


TTT.o:	TTTS.c common.h

ttt.o:	ttt.c common.h

clean:
	rm -f *.o ttt TTTS serverloc
