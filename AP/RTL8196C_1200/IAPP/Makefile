all:		iapp

CFLAGS = -s -Wall -c -Os
LDFLAGS = -s -Wall

ifeq ($(SLINK),1)
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += --static -Wl,--gc-sections
endif


iapp:		iapp.o version.o
			mips-uclibc-gcc $(LDFLAGS) -o iapp iapp.o version.o

clean:
			rm -f *.o
			rm -f iapp
			rm -f core

iapp.o:		iapp.c iapp.h common.h misc.h
			mips-uclibc-gcc $(CFLAGS) -o iapp.o iapp.c

version.o:	version.c
			mips-uclibc-gcc $(CFLAGS) -o version.o version.c
