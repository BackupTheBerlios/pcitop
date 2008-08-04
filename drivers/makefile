# Makefile for hplba module (poke and prod at HP IA-64 LBA's).

TARGETS=hplba.ko

KERNEL_ROOT_DIR=/lib/modules/`uname -r`/build


SRCS := hplba_drv.c \
        hplba.h \
        hplba_sysfs.c \
	hplba_sysfs.h \
        apollo.c \
        apollo.h \
	gemini.c \
	gemini.h \
        mercury.c \
        mercury.h

SRCS := $(strip $(SRCS))

all: $(TARGETS)

hplba.ko: $(SRCS)
	make -C $(KERNEL_ROOT_DIR) SUBDIRS=$(PWD) modules

clean:
	-rm -f *.o *.ko hplba.mod.c .*.cmd Module*.symvers modules.order
	-rm -rf .tmp_versions

FORCE:
