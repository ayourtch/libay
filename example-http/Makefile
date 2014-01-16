CC=gcc
CFLAGS=-Wall -I.. -g
LDFLAGS=-L .. -lay

OBJECTS=example-http.o http-parser.o
SOURCE=example-http.c

example-http: $(OBJECTS) http-parser.c
	$(CC) $(CFLAGS) $(OBJECTS) -o example-http $(LDFLAGS)

http-parser.c: http-parser.rl
	ragel http-parser.rl


all:example-http

.PHONY: clean
clean:
	rm -f *~ *.o example-http

