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

#include <linux/types.h>

#include "mercury.h"
#include "hplba.h"

static char *mercury_revision(void *phys_addr)
{
	static char revision[REVISION_SIZE];

	uint64_t major = LBA_REG(phys_addr, MERCURY_MAJOR_REVISION);
	uint64_t minor = LBA_REG(phys_addr, MERCURY_MINOR_REVISION);

	memset(revision, 0, REVISION_SIZE);
	snprintf(revision, REVISION_SIZE, "%lu.%lu", major,minor);
	return revision;
}

char *mercury_bus_op_mode(void *phys_addr)
{
	int bus_op_mode = LBA_UNK_TYPE;
	uint64_t bus_mode = LBA_REG(phys_addr, MERCURY_AGP_BUS_MODE);

	if (bus_mode) {
		bus_op_mode = LBA_AGP;
	} else {
		bus_mode = LBA_REG(phys_addr, MERCURY_BUS_OP_MODE);
		switch (bus_mode) {
		case 0:
			bus_op_mode = LBA_PCI;
			break;
		case 1:
		case 2:
		case 3:
			bus_op_mode = LBA_PCI_X;
			break;
		}
	} 

	return bus_op_modes_tab[bus_op_mode].name;
}
	
char *mercury_bus_speed(void *phys_addr) 
{
	int bus_speed = LBA_UNK_SPEED;
	uint64_t agp_bus_mode = LBA_REG(phys_addr, MERCURY_AGP_BUS_MODE);
	uint64_t pci_bus_mode;
	
	if (agp_bus_mode) {
		uint64_t data_rate = LBA_REG(phys_addr, MERCURY_AGP_DATA_RATE);
		switch (data_rate) {
		case 1:
			bus_speed = LBA_AGP_1X;
			break;
		case 2:
			bus_speed = LBA_AGP_2X;
			break;
		case 4:
			bus_speed = LBA_AGP_4X;
			break;
		}
	} else {
		pci_bus_mode = LBA_REG(phys_addr, MERCURY_BUS_OP_MODE);
		switch (pci_bus_mode) {
		case 0:
			bus_speed = LBA_66MHZ;
			break;
		case 1:
			bus_speed = LBA_100MHZ;
			break;
		case 2:
			bus_speed = LBA_66MHZ;
			break;
		case 3:
			bus_speed = LBA_133MHZ;
			break;
		}
	} 

	return bus_speeds_tab[bus_speed].name;
}

int mercury_ropes(void *phys_addr)
{
	if (!LBA_REG(phys_addr, MERCURY_ROPE_2X_L))
		return 2;
	else return 1;
}

static int mercury_util_init(void *phys_addr)
{
	LBA_REG_WRITE(phys_addr, MERCURY_DIAG_EN0, 0);
	LBA_REG_WRITE(phys_addr, MERCURY_CTRL0A_ALWAYS_CNT, 0x1);
	LBA_REG_WRITE(phys_addr, MERCURY_DIAG_CTRL1, 0x797900ff001f043cULL); 
	return 0;
}

static int mercury_util_reset(void *phys_addr)
{
	LBA_REG_WRITE(phys_addr, MERCURY_DIAG_EN0, 0xf);

	return 0;
}

static int mercury_util_start(void *phys_addr)
{
	LBA_REG_WRITE(phys_addr, MERCURY_DIAG_EN0, 0x3);

	return 0;
}

static int mercury_util_stop(void *phys_addr)
{
	LBA_REG_WRITE(phys_addr, MERCURY_DIAG_EN0, 0x0);
	return 0;
}

static int mercury_utilization(void *phys_addr, uint64_t *timer, uint64_t *cntr)
{
	*timer = LBA_REG(phys_addr, MERCURY_DIAG_CTRL0_TIMER);
	*cntr = LBA_REG(phys_addr, MERCURY_DIAG_CTRL1_CNTR1);

	return 0;
}

struct lba_util_ops mercury_util_ops = {
	mercury_util_init,
	mercury_util_reset,
	mercury_util_start,
	mercury_util_stop,
};

struct lba_ops mercury_ops = {
	mercury_revision,
	mercury_bus_op_mode,
	mercury_bus_speed,
	mercury_ropes,
	mercury_utilization,
	&mercury_util_ops,
};
