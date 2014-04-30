CC=gcc
CFLAGS=-Wall -I..
LDFLAGS=-L .. -lay -lpcap

OBJECTS=example-captap.o
SOURCE=example-captap.c

example-captap: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o example-captap $(LDFLAGS)
	@echo ..... Done. Need you to set capabilities on the executable, though:
	@echo sudo setcap CAP_NET_ADMIN=+ep example-captap

all:example-captap

.PHONY: clean
clean:
	rm -f *~ *.o example-captap

