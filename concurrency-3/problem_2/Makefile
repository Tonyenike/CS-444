CC=gcc --std=c99 -g -lpthread -Wall 

all: problem2

problem2: problem2.c mt.o stack.o queue.o link.h
	$(CC) problem2.c mt.o stack.o queue.o -o problem2

stack.o: stack.c stack.h link.h
	$(CC) -c stack.c -o stack.o

queue.o: queue.c queue.h link.h
	$(CC) -c queue.c -o queue.o

mt.o: mt.c mt.h
	$(CC) -c mt.c -o mt.o

clean:
	rm -rf *.dSYM/
	rm -f *.o problem2
