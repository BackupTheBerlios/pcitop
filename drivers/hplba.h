/*
 * HP IA-64 LBA driver
 *
 * Copyright (C) 2007 Hewlett-Packard Co
 * Copyright (C) 2007 Andrew Patterson <andrew.patterson@hp.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _HPLBA_H_
#define _HPLBA_H_

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

enum lba_bus_op_modes {
	LBA_AGP = 0,
	LBA_PCI,
	LBA_PCI_X,
	LBA_PCI_X_ECC,
	LBA_PCI_X2_ECC,
	LBA_PCIE,
	LBA_UNK_TYPE
};
struct bus_op_modes {
	enum lba_bus_op_modes value;
	char *name;
};
extern struct bus_op_modes bus_op_modes_tab[];

static inline 
const char *hplba_bus_op_mode_to_str(enum lba_bus_op_modes mode) {

	int i;

	for (i = 0; i <= LBA_UNK_TYPE; i++) {
		if (mode ==  bus_op_modes_tab[i].value)
			return bus_op_modes_tab[i].name;
	}
	return bus_op_modes_tab[LBA_UNK_TYPE].name;
}

static inline 
int hplba_bus_op_mode_to_value(const char *name) {

	int i, len;

	for (i = 0; i <= LBA_UNK_TYPE; i++) {
		len = strlen(bus_op_modes_tab[i].name);
		if (strncmp(bus_op_modes_tab[i].name, name, len) == 0 && 
			(name[len] == '\n' || name[len] == '\0'))
			return bus_op_modes_tab[i].value;
	}
	return LBA_UNK_TYPE;
}

enum lba_bus_speeds {
	LBA_66MHZ = 0,
	LBA_100MHZ,
	LBA_133MHZ,
	LBA_266MHZ,
	LBA_X1,
	LBA_X2,
	LBA_X4,
	LBA_X8,
	LBA_AGP_1X,
	LBA_AGP_2X,
	LBA_AGP_4X,
	LBA_UNK_SPEED
};
struct bus_speeds {
	enum lba_bus_speeds value;
	char *name;
};
extern struct bus_speeds bus_speeds_tab[];

static inline 
const char *hplba_bus_speed_to_str(enum lba_bus_speeds speed) {

	int i;

	for (i = 0; i <= LBA_UNK_SPEED; i++) {
		if (speed ==  bus_speeds_tab[i].value)
			return bus_speeds_tab[i].name;
	}
	return bus_speeds_tab[LBA_UNK_SPEED].name;
}

static inline 
int hplba_bus_speed_to_value(const char *name) {

	int i, len;

	for (i = 0; i <= LBA_UNK_SPEED; i++) {
		len = strlen(bus_speeds_tab[i].name);
		if (strncmp(bus_speeds_tab[i].name, name, len) == 0 && 
			(name[len] == '\n' || name[len] == '\0'))
			return bus_speeds_tab[i].value;
	}
	return LBA_UNK_SPEED;
}

enum lba_util_states {
	LBA_UTILSTATE_INIT = 0,
	LBA_UTILSTATE_RESET,
	LBA_UTILSTATE_START,
	LBA_UTILSTATE_STOP,
	LBA_UTILSTATE_UNK
};
struct util_states {
	enum lba_util_states value;
	char *name;
};
extern struct util_states util_states_tab[];

static inline 
const char *hplba_util_state_to_str(enum lba_util_states speed) {

	int i;

	for (i = 0; i <= LBA_UTILSTATE_UNK; i++) {
		if (speed ==  util_states_tab[i].value)
			return util_states_tab[i].name;
	}
	return util_states_tab[LBA_UTILSTATE_UNK].name;
}

static inline 
int hplba_util_state_to_value(const char *name) {

	int i, len;

	for (i = 0; i <= LBA_UTILSTATE_UNK; i++) {
		len = strlen(util_states_tab[i].name);
		if (strncmp(util_states_tab[i].name, name, len) == 0 && 
			(name[len] == '\n' || name[len] == '\0'))
			return util_states_tab[i].value;
	}
	return LBA_UTILSTATE_UNK;
}

#ifdef __KERNEL__

#include <linux/types.h>
#include <linux/device.h>
#include <linux/acpi.h>
#include <linux/version.h>

extern int pcieei_debug;

#define HPLBA_MODULE_NAME "hplba"

#define HP_LBA_PCIE_BRIDGE_PNPID "HPQ0002"
#define HP_LBA_PCIX_BRIDGE_PNPID "HWP0002"
#define HP_LBA_AGP_BRIDGE_PNPID "HWP0003"
#define HP_LBA_AGP_BRIDGE_DEV "HWP0007"

#define REVISION_SIZE 6

#define LBA_REG(phys_addr, reg) ((reg##_SIZE) == 32 ?			    \
	((readl((phys_addr) + (reg)) >> (reg##_SHIFT)) & (reg##_MASK))	    \
	:								    \
	((readq((phys_addr) + (reg)) >> (reg##_SHIFT)) & (reg##_MASK)))	

static inline void _lba_reg32_write(void *phys_addr, uint32_t offset, 
				    uint32_t shift, uint32_t mask, uint32_t value)
{
	uint32_t contents = (readl(phys_addr + offset) & ~(mask << shift))  ^ (value << shift);
	writel(contents, phys_addr + offset);
	readl(phys_addr + offset);
}
static inline void _lba_reg64_write(void *phys_addr, uint64_t offset, 
				    uint64_t shift, uint64_t mask, uint64_t value)
{
	uint64_t contents = (readq(phys_addr + offset) & ~(mask << shift))  ^ (value << shift);
	writeq(contents, phys_addr + offset);
	readq(phys_addr + offset);
}

#define LBA_REG_WRITE(phys_addr, reg, value)				    \
	do {								    \
		((reg##_SIZE) == 32) ?					    \
			_lba_reg32_write((phys_addr), (reg), (reg##_SHIFT), \
					 (reg##_MASK), value)		    \
			  :						    \
			_lba_reg64_write((phys_addr), (reg), (reg##_SHIFT), \
					 (reg##_MASK), value);		    \
	} while (0)

#define LBA_FUNC_ID	   0x00
#define LBA_FUNC_ID_SHIFT  16
#define LBA_FUNC_ID_MASK   0xffff		 
#define LBA_FUNC_ID_SIZE   64 

#define dbg(format, arg...)						   \
	do {								   \
		if (hplba_debug)					   \
			printk("%s: " format, HPLBA_MODULE_NAME , ## arg); \
	} while (0)

#define err(format, arg...)						\
	printk(KERN_ERR "%s: " format, HPLBA_MODULE_NAME , ## arg)
#define info(format, arg...)						\
	printk(KERN_INFO "%s: " format, HPLBA_MODULE_NAME , ## arg)
#define warn(format, arg...)						\
	printk(KERN_WARNING "%s: " format, HPLBA_MODULE_NAME , ## arg)
#define tracemsg(format, arg...)					\
	printk(KERN_ERR "%s:%s:%d "					\
	       format, __FILE__ , __FUNCTION__, __LINE__, ## arg)
#define trace()								\
	printk(KERN_ERR "%s:%s:%d\n",					\
	       __FILE__ , __FUNCTION__, __LINE__)

struct lba_util_ops {
	int (*util_init)(void *phys_addr);
	int (*util_reset)(void *phys_addr);
	int (*util_start)(void *phys_addr);
	int (*util_stop)(void *phys_addr);
};

struct lba_ops {
	char * (*revision)(void *phys_addr);
	char * (*bus_op_mode)(void *phys_addr);
	char * (*bus_speed)(void *phys_addr);
	int (*ropes)(void *phys_addr);
	int (*utilization)(void *phys_addr, uint64_t *timer, uint64_t *cntr);
	struct lba_util_ops *util_ops;
};

struct lba {
	struct list_head list;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	struct device dev;
#else
	struct class_device dev;
#endif
	acpi_handle *acpi_hdl;
	struct acpi_device *acpi_dev; 
	void *phys_addr;
	u16   segment;
	int util_state;
	struct lba_ops *ops;
};

struct lba_info {
	int function_id;
	struct lba_ops *ops;
};

#define to_lba(n) container_of(n, struct lba, dev)

extern int set_util_state(struct lba *lba, int new_state);

#endif /* __KERNEL__ */
#endif /* _HPLBA_H_ */
