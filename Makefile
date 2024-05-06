CC=gcc
CFLAGS=-I.

myshell.o: myshell.c
	$(CC) -c -o myshell.o myshell.c $(CFLAGS)

myshell: myshell.o
	$(CC) -o myshell myshell.o


