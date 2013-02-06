CFLAGS=-g -Wall
CC=mipsel-linux-gcc
AR=mipsel-linux-ar
RANLIB=mipsel-linux-ranlib
LIBS=-L./ -lcgic

all: libcgic.a index.cgi cgictest.cgi capture

install: libcgic.a
#	cp libcgic.a ../../toolchain/lib
#	cp cgic.h ../../toolchain/include

libcgic.a: cgic.o cgic.h
	rm -f libcgic.a
	$(AR) rc libcgic.a cgic.o
	$(RANLIB) libcgic.a

#mingw32 and cygwin users: replace .cgi with .exe

cgictest.cgi: cgictest.o libcgic.a
	$(CC) cgictest.o -o cgictest.cgi ${LIBS}

index.cgi: index.o libcgic.a
	$(CC) index.o -o index.cgi ${LIBS}

capture: capture.o libcgic.a
	$(CC) capture.o -o capture ${LIBS}

clean:
	rm -f *.o *.a *.cgi capture

