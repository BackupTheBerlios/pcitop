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

#ifndef _HPLBA_SYSFS_H_
#define _HPLBA_SYSFS_H_

#include <linux/device.h>
#include <linux/acpi.h>
#include <linux/version.h>

extern int register_hplba_class(void);
extern void unregister_hplba_class(void); 
extern int register_hplba_device(struct lba *lba, struct acpi_pci_id *pci_id );
extern void unregister_hplba_device(struct lba *lba);

extern char *get_device_id(struct lba *lba);

#endif // _HPLBA_SYSFS_H_
