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

#ifndef __PCI_SYSFS_H__
#define __PCI_SYSFS_H__

#include "hplba.h"
#include "list.h"

#define ROOT_BRIDGE_NAME_LEN  8
#define BUS_NAME_LEN  8
#define SLOT_NAME_LEN 20
#define SLOT_ADDRESS_LEN 11

#define SYSFS_SLOTS_DIR "/sys/bus/pci/slots"
#define SYSFS_DEV_DIR "/sys/devices"
#define SYSFS_BUS_DIR "/sys/class/pci_bus"

struct sysfs_root_bridge {

	char name[ROOT_BRIDGE_NAME_LEN];
	struct list_node list; 
	struct list_head slots;
};

struct sysfs_slot {
	char name[SLOT_NAME_LEN];
	struct list_node list;
	struct list_node root_bridge_slot_list;
	char address[SLOT_ADDRESS_LEN];
	struct sysfs_bus *bus;
};

struct sysfs_bus {
	char name[BUS_NAME_LEN];
	struct list_node list;
	struct sysfs_root_bridge *root_bridge;
	
};

extern int create_sysfs_pci_tree(void);
extern struct sysfs_root_bridge *get_sysfs_root_bridge(const char *bridge_name);

extern void dump_sysfs_pci_tree(void);


#endif /* __PCI_SYSFS_H__ */
