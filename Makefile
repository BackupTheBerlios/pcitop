# Makefile for hplba driver and tools used for interrogating and monitoring
# PCI root bridges and devices.

PWD	:= $(shell pwd)
SUBDIRS = drivers src

PACKAGE_NAME=pcitop
PACKAGE_VERSION=1.0.1
TARBALL=$(PACKAGE_NAME)-$(PACKAGE_VERSION).tgz
DIST_DIR=$(PACKAGE_NAME)-$(PACKAGE_VERSION)
DIST_FILES := \
	AUTHORS \
	COPYING \
	ChangeLog \
	FAQ \
	INSTALL \
	Makefile \
	NEWS \
	README \
	TODO \
	$(SUBDIRS)

prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin



all: 
	@for subdir in $(SUBDIRS); do \
		echo "Making target $@ in $$subdir"; \
		(cd $$subdir && $(MAKE) $@) \
		done

clean:
	@for subdir in $(SUBDIRS); do \
		echo "Making target $@ in $$subdir"; \
		(cd $$subdir && $(MAKE) $@) \
		done
	-rm -f $(TARBALL)
	-rm -rf $(DIST_DIR)

install:
	@for subdir in $(SUBDIRS); do \
		echo "Making target $@ in $$subdir"; \
		(cd $$subdir && $(MAKE) $@) \
		done

dist:	clean
	mkdir -p $(DIST_DIR)
	cp -a $(DIST_FILES) $(DIST_DIR)
	tar -czof $(TARBALL) $(DIST_DIR)
	-rm -rf $(DIST_DIR)

