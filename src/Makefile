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
CFLAGS=$(DEBUG) $(OPTIM) -I../drivers -D_GNU_SOURCE
LDFLAGS=-Wl,-E -ldl

prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin

all: $(TARGETS)

install: all 
	mkdir -p ${bindir}
	cp -av ${PRODACRO} ${bindir}

pcitop.o: pcitop.c pcitop.h

pcitop: pcitop.c
	$(CC) -o $@ pcitop.c $(CFLAGS) $(LDFLAGS)

clean:
	-rm -f *.o pcitop 

