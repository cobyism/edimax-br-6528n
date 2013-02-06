
IPOBJ=ip.o iproute.o iprule.o ipaddress.o iplink.o\
    rtm_map.o  
    

RTMONOBJ=rtmon.o

ALLOBJ=$(IPOBJ) $(RTMONOBJ)

TARGETS=ip rtmon

all: $(TARGETS)
	$(STRIP) ip

ip: $(IPOBJ) $(LIBNETLINK) $(LIBUTIL)

rtmon: $(RTMONOBJ) $(LIBNETLINK)

install: all
	install -m 0755 -s $(TARGETS) $(DESTDIR)$(SBINDIR)
	install -m 0755 routel routef $(DESTDIR)$(SBINDIR)

clean:
	rm -f $(ALLOBJ) $(TARGETS)

