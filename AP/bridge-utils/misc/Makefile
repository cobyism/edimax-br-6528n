include ../../../make.def

ifeq ($(MODEL),-D_7206GA_)
	CFLAGS=-Wall -g --static
else
	CFLAGS=
endif

all:		bidi

clean:
		rm -f bidi

bidi:		bidi.c
		$(CC) $(CFLAGS) -o $@ $<
