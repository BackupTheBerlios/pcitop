# Makefile for pcitop utilty for interrogating and monitoring
# PCI root bridges and devices.

TARGETS=pcitop

PRODNAME= PCI Monitor
PRODACRO= pcitop
PRODVER= 
PRODREL= 1.0
PWD	:= $(shell pwd)

DEBUG=-g -W -Wall
#OPTIM=-O2
CPPFLAGS=-I../drivers -D_GNU_SOURCE
CFLAGS=$(DEBUG)$(OPTIM) $(CPPFLAGS)
LDFLAGS=-Wl,-E -ldl
MAKEDEPEND=$(CC) -MM $(CPPFLAGS)

prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin

SRCS := \
	pcitop.c \
	pci_sysfs.c \
	util.c \
	list.c \
	noerr.c \
	string.c \
	talloc.c

SRCS := $(strip $(SRCS))

OBJS := $(subst .c,.o,$(SRCS))

all: $(TARGETS)

install: all 
	mkdir -p ${bindir}
	cp -av ${PRODACRO} ${bindir}

%.o:	%.c
	$(CC) -c $< $(CFLAGS)
	$(MAKEDEPEND) -o $*.d $<
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

pcitop: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	-rm -f *.o *.d *.P pcitop 

-include $(SRCS:.c=.P)