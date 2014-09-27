CC=gcc
CFLAGS=-Wall -I.. -I./lua-5.1.4/src -I. -g
LDFLAGS=-L .. -L./lua-5.1.4/src -lay -lssl -lcrypto -llua -lm -lsqlite3
all: example-http


OBJECTS=example-http.o http-parser.o lsqlite3.o
SOURCE=example-http.c

lua-5.1.4/src/lua: lua-5.1.4.tar.gz
	tar xzvf lua-5.1.4.tar.gz
	(cd lua-5.1.4; make posix)

example-http: $(OBJECTS) http-parser.c data.zip ../libay.a lua-5.1.4/src/lua lsqlite3.o
	$(CC) $(CFLAGS) $(OBJECTS) -o example-http $(LDFLAGS)

.PHONY: data.zip
data.zip:
	(cd data; zip -r ../data.zip .)

http-parser.c: http-parser.rl
	ragel http-parser.rl


all:example-http

.PHONY: clean
clean:
	rm -f *~ *.o example-http

