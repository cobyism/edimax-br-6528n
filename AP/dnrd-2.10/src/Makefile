#sc_yang
#ENABLE_CACHE=1
#PTHREAD_LIB=1

include ../../../make.def
include ../../../define/FUNCTION_COMPILER
#CC		= mips-uclibc-gcc
COPT		= -O2
CDEF		= -D_REENTRANT
#CWARN		= -Wall -Werror
CWARN		=
CDBG		= -ggdb
#CFLAGS		= ${CWARN} ${COPT} ${CDEF} ${CDBG} -DCOMPACK_SIZE
CFLAGS         = ${CWARN} ${COPT} ${CDEF} ${CDBG}
#ifdef ENABLE_CACHE
CFLAGS += -DENABLE_CACHE
CFLAGS += $(FUNCTION)

#endif
LDFLAGS		= 
ifdef PTHREAD_LIB
LIBS            = -lpthread
CFLAGS += -DPTHREAD_LIB
endif
INSTDIR		= /usr/local/sbin
MANDIR		= /usr/local/man/man8
SHELL		= /bin/sh
#STRIP		= mips-uclibc-strip

ifeq ($(SLINK),1)
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += --static -Wl,--gc-sections
endif


all:	dnrd
	-ctags *.[ch]

clean:
	rm -f dnrd core tags ID *.o *.d *~ *.bak

install: dnrd
	strip dnrd
	install -m 0751 -o root -g root -s dnrd ${INSTDIR}
	install -m 0644 -o root -g root -s ../doc/dnrd.8 ${MANDIR}

sources = args.c common.c dns.c lib.c main.c \
	  query.c relay.c sig.c tcp.c udp.c

#ifdef ENABLE_CACHE
sources += cache.c master.c
#endif

include $(sources:.c=.d)

dnrd:	$(sources:.c=.o)
	$(CC) $(LDFLAGS) $(LIBS) $^ -o $@	
	$(STRIP) $@

%.d: %.c
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
		      | sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		      [ -s $@ ] || rm -f $@'
