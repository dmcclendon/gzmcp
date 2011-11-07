#
# Makefile for the Guitar-ZyX Master Control Program
#

####################
# Global Variables #
####################
TOP         = .
include $(TOP)/build/makefile.common


# nothing to make in tools
#	tools 
SUBDIRS = \
	server-pc \
	client-pc-debug \
	client-nds \

###############
# Build Rules #
###############

default:
	make all

all clean install uninstall: $(SUBDIRS)
	for subdir in $(SUBDIRS); do \
	(cd $${subdir}; $(MAKE) $@); \
	make $@_also; \
	done

all_also:

clean_also:

install_also:
	mkdir -p $(PREFIX)/lib/gzmcp/ 
	mkdir -p $(PREFIX)/share/doc/
	cp -rv ./doc $(PREFIX)/share/doc/gzmcp-$(VERSION)
	mkdir -p $(PREFIX)/bin

uninstall_also:
	rm -rvf $(PREFIX)/lib/gzmcp
	rm -rvf $(PREFIX)/share/doc/gzmcp-$(VERSION)
	
tidy:
	@ echo "removing temporary and backup files"
	find . -name "*~" -exec rm -vf '{}' ';'
	find . -name "#*" -exec rm -vf '{}' ';'
	find . -name "*.d.*" -exec rm -vf '{}' ';'

release: 
	make tidy
	make clean
	@ echo "building release tarball - v$(VERSION) r$(RELEASE)"
	./tools/makerelease $(VERSION) $(RELEASE)
	sha512sum gzmcp-$(VERSION)* > gzmcp-$(VERSION).sha512sums

xrelease:
	make release
	tar xvjf gzmcp-$(VERSION).tar.bz2

distclean:
	make tidy
	make clean
	rm -f gzmcp-$(VERSION)-$(RELEASE).src.rpm 
	rm -f gzmcp-$(VERSION)-$(RELEASE).i386.rpm 
	rm -f gzmcp-$(VERSION).tar.bz2
	rm -rf gzmcp-$(VERSION)

srpm:	
	rpmdev-setuptree
	make release
	cp gzmcp-$(VERSION).tar.bz2 ${HOME}/rpmbuild/SOURCES/
	rpmbuild -bs build/gzmcp.spec
	mv ${HOME}/rpmbuild/SRPMS/gzmcp-$(VERSION)-$(RELEASE).src.rpm .

rpm:
	make srpm
	rpm -i gzmcp-$(VERSION)-$(RELEASE).src.rpm 
	rpmbuild --rebuild gzmcp-$(VERSION)-$(RELEASE).src.rpm 
	mv ${HOME}/rpmbuild/RPMS/i386/gzmcp-$(VERSION)-$(RELEASE).i386.rpm .

