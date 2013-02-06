CC		= gcc
COPT		= -O2
CDEF		= -D_REENTRANT
CWARN		= -Wall -Werror
CDBG		= -ggdb
CFLAGS		= ${CWARN} ${COPT} ${CDEF} ${CDBG}
LDFLAGS		= 
LIBS            = -lpthread
INSTDIR		= /usr/local/sbin
MANDIR		= /usr/local/man/man8
SHELL		= /bin/sh

all:	dnrd
	-ctags *.[ch]

clean:
	rm -f dnrd core tags ID *.o *.d *~ *.bak

install: dnrd
	strip dnrd
	install -m 0751 -o root -g root -s dnrd ${INSTDIR}
	install -m 0644 -o root -g root -s ../doc/dnrd.8 ${MANDIR}

sources = args.c cache.c common.c dns.c lib.c main.c master.c \
	  query.c relay.c sig.c tcp.c udp.c

include $(sources:.c=.d)

dnrd:	$(sources:.c=.o)
	$(CC) $(LDFLAGS) $(LIBS) $^ -o $@

%.d: %.c
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
		      | sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		      [ -s $@ ] || rm -f $@'
