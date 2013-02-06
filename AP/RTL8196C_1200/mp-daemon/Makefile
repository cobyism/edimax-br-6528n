all:	UDPserver

CC	= mips-uclibc-gcc
STRIP	= mips-uclibc-strip

CFLAGS	= 
IFLAGS	=
LDFLAGS =

ifeq ($(SLINK),1)
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += --static -Wl,--gc-sections
endif


DEBUG	= -Wall -Os

UDPserver: Makefile UDPserver.o
	$(CC) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $(LDFLAGS) UDPserver.o
	$(STRIP) $@

clean:
	rm -f UDPserver *.o 

UDPserver.o: UDPserver.c
	$(CC) -c -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $<
