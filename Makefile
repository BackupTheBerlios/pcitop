# Makefile for hplba driver and tools used for interrogating and monitoring
# PCI root bridges and devices.

prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin

PWD	:= $(shell pwd)
SUBDIRS = drivers src

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
install:
	@for subdir in $(SUBDIRS); do \
		echo "Making target $@ in $$subdir"; \
		(cd $$subdir && $(MAKE) $@) \
		done
