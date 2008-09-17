# Remove or comment out the following ifdef's if building in tree.
ifdef CONFIG_IA64_GENERIC
CONFIG_HPLBA=m
endif
ifdef CONFIG_IA64_HP_ZX1
CONFIG_HPLBA=m
endif

obj-$(CONFIG_HPLBA) += hplba.o

hplba-objs := hplba_drv.o hplba_sysfs.o apollo.o gemini.o mercury.o

