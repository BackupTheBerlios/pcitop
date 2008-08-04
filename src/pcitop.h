/*
 * HP IA-64 LBA utilization monitor
 *
 * Copyright (C) 2007 Hewlett-Packard Co
 * Copyright (C) 2007 Andrew Patterson <andrew.patterson@hp.com>
 * Copyright (C) 2007 Rick Jones <rick.jones2@hp.com>
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

#ifndef __PCITOP_H__
#define __PCITOP_H__

#define DEFAULT_DEBUG 1
#ifdef DEFAULT_DEBUG
#define DPRINT(a)						   \
	do {							   \
		if (options.opt_debug >0) {			   \
			printf("%s.%d: ", __FUNCTION__, __LINE__); \
			printf a;				   \
		}						   \
	} while (0)
#else
#define DPRINT(a)
#endif

#define TRACE() printf("%s:%d\n", __func__, __LINE__)
#define TRACE_MSG(fmt, arg...) printf("%s:%d: " fmt, __func__, __LINE__, ##arg)

#define REV_LEN 10
#define SLOT_NAME_SIZE 20	/* from drivers/acpi/pci_slot.c */
#define ITANIUM_CABINET_NAME_SIZE 2
#define ITANIUM_CHASSIS_NAME_SIZE 2
#define ITANIUM_BAY_NAME_SIZE 2
#define ITANIUM_SLOT_NAME_SIZE 2

typedef unsigned long long u64;

struct pci_id {
	unsigned int pci_domain;
	unsigned int pci_busid;
	unsigned int pci_addr;
	unsigned int pci_func;
};

typedef struct {
	int			opt_verbose;
	int			opt_debug;
	int			opt_show_info;
	int			opt_match_bus;
	int			opt_match_domain;
	int			opt_match_and;
	int			opt_match_or;
	char *			opt_match_cabinet;
	char *			opt_match_bay;
	char *			opt_match_chassis;
	char *			opt_match_slot;
	char *			opt_match_lspci;
	unsigned long		num_lba;
	unsigned long		num_lba_display;
	unsigned long		nsamples;
	int			timeout;
} pcitop_options_t;

struct slot {
	struct slot *next;
	char *name;
};

struct lba_info {
	struct lba_info *next;
	const char *	name;
	const struct lba_ops *ops;
	unsigned long	prev_timer;	   /* old value */
	unsigned long	prev_counter;	   /* old value */
	unsigned long	timer;		   /* new value */
	unsigned long	counter;	   /* new value */
	const char *	bus_type; 
	const char *	bus_speed;
	char revision[REV_LEN];		   /* chip revision */
	unsigned int id;		   /* chip id */
	unsigned int ropes;
	struct pci_id pci_id;		   /* domain, bus, addr, func */
	unsigned int in_use;		   /* any devices behind lba */
	unsigned int display;		   /* do we want to see it or not */
	char cabinet[ITANIUM_CABINET_NAME_SIZE];  /* the id of the cabinet W */
	char chassis[ITANIUM_CHASSIS_NAME_SIZE];  /* the id of the chassis Y */
	char bay[ITANIUM_BAY_NAME_SIZE];	  /* the id of the bay X */
	struct slot *slot;	 /* a linked list of the ids of the slots ZZ */
};

struct lba_ops {
	int (*init)(struct lba_info *lba);
	int (*reset)(struct lba_info *lba);
	int (*start)(struct lba_info *lba);
	int (*stop)(struct lba_info *lba);
	int (*collect)(struct lba_info *lba);
};

#endif /* __PCITOP_H__ */
