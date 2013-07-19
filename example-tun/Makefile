CC=gcc
CFLAGS=-Wall -I..
LDFLAGS=-L .. -lay

OBJECTS=example-tun.o
SOURCE=example-tun.c

example-tun: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o example-tun $(LDFLAGS)
	@echo ..... Done. Need you to set capabilities on the executable, though:
	@echo sudo setcap CAP_NET_ADMIN=+ep example-tun

all:example-tun

.PHONY: clean
clean:
	rm -f *~ *.o example-tun

