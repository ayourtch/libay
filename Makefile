all: test
CFLAGS=-g

CC=gcc
OBJS=debug-ay.o lists-ay.o hash-func-ay.o hash-ay.o uuid_sysdep.o uuid-ay.o dbuf-ay.o timers-ay.o os-ay.o sock-ay.o httpd-ay.o

debug-ay.o: debug-ay.c debug-ay.h
lists-ay.o: lists-ay.c lists-ay.h debug-ay.o
hash-func-ay.o: hash-func-ay.c hash-func-ay.h
hash-ay.o: hash-ay.c hash-ay.h hash-func-ay.o debug-ay.o
uuid_sysdep.o: uuid_sysdep.c uuid_sysdep.h
uuid-ay.o: uuid-ay.c uuid-ay.h uuid_sysdep.o
dbuf-ay.o: dbuf-ay.c dbuf-ay.h
timers-ay.o: timers-ay.c timers-ay.h dbuf-ay.o uuid-ay.o lists-ay.o debug-ay.o
os-ay.o: os-ay.c os-ay.h debug-ay.o
sock-ay.o: sock-ay.c sock-ay.h
httpd-ay.o: httpd-ay.c httpd-ay.h sock-ay.o hash-ay.o debug-ay.o lists-ay.o



libay.a: ${OBJS}
	ar rcs libay.a ${OBJS}
libay.so: ${OBJS}
	gcc -shared -Wl,-soname,libay.so.1 -o libay.so.1.0.1 ${OBJS}
test: test.c libay.a
	${CC} -g -o test test.c libay.a -lssl
clean:
	rm -f *.o
	rm -f *.a
	rm -f test


