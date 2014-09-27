CC=gcc
CFLAGS=-Wall -I.. -g
LDFLAGS=-L .. -lay

OBJECTS=example-tftp.o 
SOURCE=example-tftp.c

example-tftp: $(OBJECTS) 
	$(CC) $(CFLAGS) $(OBJECTS) -o example-tftp $(LDFLAGS)

all:example-tftp

.PHONY: clean
clean:
	rm -f *~ *.o example-tftp

