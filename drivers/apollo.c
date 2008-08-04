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

#include "apollo.h"
#include "hplba.h"

static char *apollo_revision(void *phys_addr)
{
	static char revision[REVISION_SIZE];

	uint64_t major = LBA_REG(phys_addr, APOLLO_MAJOR_REVISION);
	uint64_t minor = LBA_REG(phys_addr, APOLLO_MINOR_REVISION);

	memset(revision, 0, REVISION_SIZE);
	snprintf(revision, REVISION_SIZE, "%lu.%lu", major,minor);
	return revision;
}

static char *apollo_bus_op_mode(void *phys_addr)
{
	return bus_op_modes_tab[LBA_PCIE].name;
}

static char *apollo_bus_speed(void *phys_addr) 
{
	char *name = NULL;
	unsigned int width = LBA_REG(phys_addr, APOLLO_MAX_LINK_WIDTH);
	
	switch (width) {
	case 0:
		name = bus_speeds_tab[LBA_X8].name;
		break;
	case 1:
		name = bus_speeds_tab[LBA_X1].name;
		break;
	case 2:
		name = bus_speeds_tab[LBA_X2].name;
		break;
	case 3:
		name = bus_speeds_tab[LBA_X4].name;
		break;
	}
	return name;
}

static int apollo_ropes(void *phys_addr)
{
	if (!LBA_REG(phys_addr, APOLLO_ROPE_4X_L))
		return 4;
	else if (!LBA_REG(phys_addr, APOLLO_ROPE_2X_L))
		return 2;
	else return 1;
}

static int apollo_util_init(void *phys_addr)
{
	LBA_REG_WRITE(phys_addr, APOLLO_DIAG_EN0, 0);
	LBA_REG_WRITE(phys_addr, APOLLO_CTRL0A_ALWAYS_CNT, 0x1);
	LBA_REG_WRITE(phys_addr, APOLLO_DIAG_CTRL1, 0x3); 

	return 0;
}

static int apollo_util_reset(void *phys_addr)
{
	LBA_REG_WRITE(phys_addr, APOLLO_DIAG_EN0, 0xf);

	return 0;
}

static int apollo_util_start(void *phys_addr)
{
	LBA_REG_WRITE(phys_addr, APOLLO_DIAG_EN0, 0x3);

	return 0;
}

static int apollo_util_stop(void *phys_addr)
{
	LBA_REG_WRITE(phys_addr, APOLLO_DIAG_EN0, 0x0);

	return 0;
}

static int apollo_utilization(void *phys_addr, uint64_t *timer, uint64_t *cntr)
{
	*timer = LBA_REG(phys_addr, APOLLO_DIAG_CTRL0_TIMER);
	*cntr = LBA_REG(phys_addr, APOLLO_DIAG_CTRL1_CNTR1);

	return 0;
}

struct lba_util_ops apollo_util_ops = {
	apollo_util_init,
	apollo_util_reset,
	apollo_util_start,
	apollo_util_stop,
};

struct lba_ops apollo_ops = {
	apollo_revision,
	apollo_bus_op_mode,
	apollo_bus_speed,
	apollo_ropes,
	apollo_utilization,
	&apollo_util_ops,
};


