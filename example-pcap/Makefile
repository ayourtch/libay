CC=gcc
CFLAGS=-Wall -I..
LDFLAGS=-L .. -lay -lpcap

OBJECTS=example-pcap.o
SOURCE=example-pcap.c


example-pcap: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o example-pcap $(LDFLAGS)

example-pcap.c: example-pcap.rl
	ragel example-pcap.rl

all:example-pcap

.PHONY: clean
clean:
	rm -f *~ *.o example-pcap

