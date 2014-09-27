CC=gcc
CFLAGS=-Wall -I.. -g
LDFLAGS=-L .. -lay

OBJECTS=example-http.o http-parser.o
SOURCE=example-http.c


example-http: $(OBJECTS) http-parser.c data.zip ../libay.a
	$(CC) $(CFLAGS) $(OBJECTS) -o example-http $(LDFLAGS)

data.zip: data
	(cd data; zip -r ../data.zip .)

http-parser.c: http-parser.rl
	ragel http-parser.rl


all:example-http

.PHONY: clean
clean:
	rm -f *~ *.o example-http

