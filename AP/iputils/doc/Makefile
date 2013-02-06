DOCDIR=/usr/doc/iputils
HTMLFILES=$(subst .sgml,.html,$(shell echo *.sgml))
MANFILES=$(subst .sgml,.8,$(shell echo *.sgml))

all: html

html: $(HTMLFILES) iputils.html index.html

man: $(MANFILES)

# This Makefile is wrong. I still did not find a way to organize
# dependencies right. We have several sources, several output
# files and all the output is produced by a single run of docbook.
# It does not look a rocket science, but I honestly still did not
# guess right combination. 

# docbook scripts are incredibly dirty in the sense that they leak
# lots of some strange temporary junk directories and files.
# So, scope it to a temporary dir and clean all after each run.

$(HTMLFILES) index.html: index.db
	@-rm -rf tmp.db2html
	@mkdir tmp.db2html
	@set -e; cd tmp.db2html; docbook2html ../$< ; mv *.html ..
	@-rm -rf tmp.db2html

iputils.html: iputils.db
	@-rm -rf tmp.db2html
	@mkdir tmp.db2html
	@set -e; cd tmp.db2html; docbook2html -u -o html ../$< ; mv html/$@ ..
	@-rm -rf tmp.db2html

# docbook2man produces utterly ugly output and I didi not find
# any way to customize this but hacking backend perl script a little.
# Well, hence...

$(MANFILES): index.db
	@-mkdir tmp.db2man
	@set -e; cd tmp.db2man; nsgmls ../$< | sgmlspl ../docbook2man-spec.pl ;	mv $@ ..
	@-rm -rf tmp.db2man

clean:
	rm -rf $(MANFILES) $(HTMLFILES) index.html iputils.html tmp.db2html tmp.db2man

snapshot:
	@date "+%y%m%d" > snapshot.db

install: html
	mkdir -p $(DESTDIR)$(DOCDIR)
	install -m 0644 $(HTMLFILES) iputils.html index.html $(DESTDIR)$(DOCDIR)
