# $Id: Makefile,v 1.2 2005/05/01 10:35:36 rex Exp $
VERSION=1.3.1
RELEASE=

#################################################################
# CHANGE THIS LINE to point to the location of your pppd binary.
PPPD = /usr/sbin/pppd
#################################################################

BINDIR=$(DESTDIR)/usr/sbin
MANDIR=$(DESTDIR)/usr/share/man/man8

include ../../make.def

#CC	= gcc
#CC	= mips-uclibc-gcc
#STRIP	= mips-uclibc-strip

RM	= rm -f
OPTIMIZE= -Os
DEBUG	=
INCLUDE =
CFLAGS  = -Wall $(OPTIMIZE) $(INCLUDE) -DNO_LD_SHARELIB #-ffunction-sections -fdata-sections
#LIBS	= -lutil
LDFLAGS	= #--static -W1,--gc-sections
ifeq ($(SLINK),1)
CFLAGS  += -DCOMPACK_SIZE -ffunction-sections -fdata-sections
LDFLAGS += --static -Wl,--gc-sections
endif

PPTP_BIN = pptp

PPTP_OBJS = pptp.o pptp_gre.o ppp_fcs.o \
            pptp_ctrl.o dirutil.o vector.o \
            inststr.o util.o version.o \
	    pptp_quirks.o orckit_quirks.o pqueue.o pptp_callmgr.o

PPTP_DEPS = pptp_callmgr.h pptp_gre.h ppp_fcs.h util.h \
	    pptp_quirks.h orckit_quirks.h config.h pqueue.h

all: config.h $(PPTP_BIN)

$(PPTP_BIN): $(PPTP_OBJS) $(PPTP_DEPS)
	$(CC) $(CFLAGS) -o $(PPTP_BIN) $(PPTP_OBJS) $(LDFLAGS) $(LIBS)
	$(STRIP) $@
 

config.h: 
	echo "/* text added by Makefile target config.h */" > config.h
	echo "#define PPTP_LINUX_VERSION \"$(VERSION)$(RELEASE)\"" >> config.h
	echo "#define PPPD_BINARY \"$(PPPD)\"" >> config.h

vector_test: vector_test.o vector.o
	$(CC) -o vector_test vector_test.o vector.o

clean:
	$(RM)  $(PPTP_BIN) *.o config.h

clobber: clean
	$(RM) $(PPTP_BIN) vector_test

distclean: clobber

test: vector_test

install:
	mkdir -p $(BINDIR)
	install -o root -m 555 pptp $(BINDIR)
	mkdir -p $(MANDIR)
	install -m 644 pptp.8 $(MANDIR)

dist: clobber
	$(RM) pptp-linux-$(VERSION)$(RELEASE).tar.gz
	$(RM) -r pptp-linux-$(VERSION)
	mkdir pptp-linux-$(VERSION)
	cp --recursive ChangeLog Makefile *.c *.h pptp.8 Documentation \
Reference AUTHORS COPYING INSTALL NEWS README DEVELOPERS TODO USING \
	pptp-linux-$(VERSION)/
	rm -rf pptp-linux-$(VERSION)/CVS pptp-linux-$(VERSION)/*/CVS
	tar czf pptp-linux-$(VERSION)$(RELEASE).tar.gz pptp-linux-$(VERSION)
	$(RM) -r pptp-linux-$(VERSION)
	md5sum pptp-linux-$(VERSION)$(RELEASE).tar.gz
