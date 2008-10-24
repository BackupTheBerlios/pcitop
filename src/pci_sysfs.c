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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "pci_sysfs.h"
#include "hplba.h"
#include "util.h"
#include "list.h"
#include "string.h"

static LIST_HEAD(root_bridge_list);
static LIST_HEAD(bus_list);
static LIST_HEAD(slot_list);

static int create_root_bridge_list(void)
{
	int ret = 1;
	DIR *dev_dir;
	struct dirent *dev_dp;
	unsigned int domain, bus_id;
	int items;
	struct sysfs_root_bridge *root_bridge;

	dev_dir = opendir(SYSFS_DEV_DIR);
	if (!dev_dir) {
		error("could not open %s\n", SYSFS_DEV_DIR);
		goto out;
	}

	while ((dev_dp = readdir(dev_dir)) != NULL) {
		if (strlen(dev_dp->d_name) == 0 ||
		    strcmp(dev_dp->d_name,".") == 0 ||
		    strcmp(dev_dp->d_name,"..") == 0)
			continue;
		items = sscanf(dev_dp->d_name, "pci%4x:%2x", &domain, &bus_id);
		if (items != 2)
			continue;
		root_bridge = calloc(1, sizeof(struct sysfs_root_bridge));
		if (!root_bridge) {
			error("could not allocate memory\n");
			goto dir;
		}
		snprintf(root_bridge->name, ROOT_BRIDGE_NAME_LEN,
			 "%04x:%02x", domain, bus_id);
		list_head_init(&root_bridge->slots);
		list_add_tail(&root_bridge_list, &root_bridge->list);
	}
	ret = 0;

dir:
	closedir(dev_dir);
out:
	return ret;
}



static int create_pci_bus_list(void)
{
	int ret = 1;
	DIR *bus_dir;
	struct dirent *bus_dp;
	int items;
	char device_path[PATH_MAX];
	char device_link[PATH_MAX];
	char root_bridge_name[PATH_MAX];
	struct sysfs_bus *bus;
	struct sysfs_root_bridge *root_bridge;
	int count;
	struct stat statbuf;

	bus_dir = opendir(SYSFS_BUS_DIR);
	if (!bus_dir) {
		error("could not open %s\n", SYSFS_BUS_DIR);
		goto out;
	}

	while ((bus_dp = readdir(bus_dir)) != NULL) {
		if (strlen(bus_dp->d_name) == 0 ||
		    strcmp(bus_dp->d_name,".") == 0 ||
		    strcmp(bus_dp->d_name,"..") == 0)
			continue;
		snprintf(device_path, PATH_MAX, "%s/%s/%s", 
			 SYSFS_BUS_DIR, bus_dp->d_name, "device");
		if (stat(device_path, &statbuf) < 0) {
			snprintf(device_path, PATH_MAX, "%s/%s/%s", 
				 SYSFS_BUS_DIR, bus_dp->d_name, "bridge");
		}
		count = readlink(device_path, device_link, PATH_MAX - 1);
		if (count < 0)
			continue;
		*(device_link + count) = '\0';
		items = sscanf(device_link, "../../../devices/pci%7s",
			       root_bridge_name);
		if (items != 1)
			continue;
		bus = calloc(1, sizeof(struct sysfs_bus));
		if (!bus) {
			error("could not allocate memory\n");
			goto dir;
		}
		strncpy(bus->name, bus_dp->d_name, BUS_NAME_LEN);
		list_for_each(&root_bridge_list, root_bridge, list) {
			if (strneq(root_bridge_name, root_bridge->name,
				   ROOT_BRIDGE_NAME_LEN)) {
				bus->root_bridge = root_bridge;
				break;
			}
		}
		list_add_tail(&bus_list, &bus->list);
		
	}
	ret = 0;

dir:
	closedir(bus_dir);
out:
	return ret;
}

static int create_sysfs_slot_list(void)
{
	int ret = 1;
	DIR *slots_dir;
        struct dirent *slot_dp;
        char address_path[PATH_MAX];
        char address[SLOT_ADDRESS_LEN];
	struct sysfs_slot *slot;
	struct sysfs_bus *bus;
	int found;

	slots_dir = opendir(SYSFS_SLOTS_DIR);
        if (!slots_dir) {
                error("could not open %s\n", SYSFS_SLOTS_DIR);
                goto out;
        }
        while ((slot_dp = readdir(slots_dir)) != NULL) {
                if (strlen(slot_dp->d_name) == 0 ||
                    streq(slot_dp->d_name,".") ||
                    streq(slot_dp->d_name,".."))
                        continue;
                snprintf(address_path, PATH_MAX, "%s/%s/address", 
                         SYSFS_SLOTS_DIR, slot_dp->d_name);
                read_attribute(address_path, address, SLOT_ADDRESS_LEN);
		
		slot = calloc(1, sizeof(struct sysfs_slot));
		if (!slot) {
			error("could not allocate memory\n");
			goto dir;
		}
		strncpy(slot->name, slot_dp->d_name, SLOT_NAME_LEN);
		found = 0;
		list_for_each(&bus_list, bus, list) {
			struct sysfs_root_bridge *bridge;
			if (strneq(address, bus->name, BUS_NAME_LEN - 1)) {
				slot->bus = bus;
				list_add_tail(&slot_list, &slot->list);
				bridge = slot->bus->root_bridge;
				list_add_tail(&bridge->slots, 
					      &slot->root_bridge_slot_list);
				found = 1;
				break;
			}
		}
		if (!found) {
			error("could not find bus for slot %s\n", slot->name);
			goto dir;
		}
	}
	ret = 0;

dir:
	closedir(slots_dir);

out:
	return ret;
}

int create_sysfs_pci_tree(void) 
{
	create_root_bridge_list();
	create_pci_bus_list();
	create_sysfs_slot_list();

	return 0;
}

extern struct sysfs_root_bridge *get_sysfs_root_bridge(const char *bridge_name)
{
	int found = 0;
	struct sysfs_root_bridge *bridge;

	list_for_each(&root_bridge_list, bridge, list) {
		if (strneq(bridge_name, bridge->name, 
			   ROOT_BRIDGE_NAME_LEN - 1)) {
			found = 1;
			break;
		}
	}
	
	if (found)
		return bridge;
	else
		return NULL;

}

void dump_sysfs_pci_tree(void)
{
	struct sysfs_root_bridge *root_bridge;
	struct sysfs_bus *bus;
	struct sysfs_slot *slot;

	list_for_each(&root_bridge_list, root_bridge, list) {
		note("bridge = %s\n", root_bridge->name);
		list_for_each(&root_bridge->slots, slot, 
			      root_bridge_slot_list) {
			note("\tslot %s\n", slot->name);
		}
	}
	list_for_each(&bus_list, bus, list) {
		note("bus = %s, root = %s\n", bus->name, 
		     bus->root_bridge->name);
		
	}
	list_for_each(&slot_list, slot, list) {
		note("slot = %s, bus = %s, root = %s\n", 
		     slot->name,
		     slot->bus->name, 
		     slot->bus->root_bridge->name);
	}
}
