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
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/acpi.h>
#include <linux/list.h>

#include <asm/acpi-ext.h>
#include <asm/uaccess.h>
#include <asm/system.h>

#include "hplba.h"
#include "hplba_sysfs.h"
#include "apollo.h"
#include "gemini.h"
#include "mercury.h"

static int hplba_debug;
static LIST_HEAD(lba_list);
static DECLARE_MUTEX(lba_list_sem);

static char *pnpids[] = {
	HP_LBA_PCIE_BRIDGE_PNPID,
	HP_LBA_PCIX_BRIDGE_PNPID,
	HP_LBA_AGP_BRIDGE_PNPID,
	HP_LBA_AGP_BRIDGE_DEV,
	NULL
};

struct bus_op_modes bus_op_modes_tab[]= {
	{LBA_AGP,"AGP"},
	{LBA_PCI,"PCI"},
	{LBA_PCI_X,"PCI-X mode 1"},
	{LBA_PCI_X_ECC,"mode 1 ECC"},
	{LBA_PCI_X2_ECC,"PCI-X mode 2 ECC"},
	{LBA_PCIE,"PCIe"},
	{LBA_UNK_TYPE,"unknown"}
};

struct bus_speeds bus_speeds_tab[] = {
	{LBA_66MHZ, "66 MHz"},
	{LBA_100MHZ, "100 MHz"},
	{LBA_133MHZ, "133 MHz"},
	{LBA_266MHZ, "266 MHz"},
	{LBA_X1, "x1"},
	{LBA_X2, "x2"},
	{LBA_X4, "x4"},
	{LBA_X8, "x8"},
	{LBA_AGP_1X, "1x"},
	{LBA_AGP_2X, "2x"},
	{LBA_AGP_4X, "4x"},
	{LBA_UNK_SPEED, "unknown"}
};

struct util_states util_states_tab[] = {
	{LBA_UTILSTATE_INIT, "init"},
	{LBA_UTILSTATE_RESET, "reset"},
	{LBA_UTILSTATE_START, "start"},
	{LBA_UTILSTATE_STOP, "stop"},
	{LBA_UTILSTATE_UNK, "unknown"}
};

struct lba_info lba_info_tab[] = {
	{APOLLO_FUNC_ID, &apollo_ops},
	{GEMINI_FUNC_ID, &gemini_ops},
	{MERCURY_FUNC_ID, &mercury_ops},
};


/*
 * Version Information
 */
#define DRIVER_VERSION "v1.0.1"
#define DRIVER_AUTHOR "Andrew Patterson <andrew.patterson@hp.com>"
#define DRIVER_DESC "Hewlett-Packard IPF Local Bus Adapter driver"
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

module_param(hplba_debug, bool, 0644);
MODULE_PARM_DESC(hplba_debug, "Debugging mode enabled or not");


int set_util_state(struct lba *lba, int new_state)
{
	int curr_state = lba->util_state;
	int ret = 0;

	switch (new_state) {
	case LBA_UTILSTATE_INIT:
		switch (curr_state) {
		case LBA_UTILSTATE_UNK:
		case LBA_UTILSTATE_INIT:
		case LBA_UTILSTATE_RESET:
		case LBA_UTILSTATE_START:
		case LBA_UTILSTATE_STOP:
			break;
		default:
			ret = -EINVAL;
		}
		break;
	case LBA_UTILSTATE_RESET:
		switch (curr_state) {
		case LBA_UTILSTATE_INIT:
		case LBA_UTILSTATE_RESET:
		case LBA_UTILSTATE_START:
		case LBA_UTILSTATE_STOP:
			break;
		default:
			ret = -EINVAL;
		}
		break;
	case LBA_UTILSTATE_START:
		switch (curr_state) {
		case LBA_UTILSTATE_UNK:
		case LBA_UTILSTATE_INIT:
			ret = -EINVAL;
			break;
		case LBA_UTILSTATE_RESET:
		case LBA_UTILSTATE_START:
		case LBA_UTILSTATE_STOP:
			break;
		default:
			ret = -EINVAL;
		}
		break;
	case LBA_UTILSTATE_STOP:
		switch (curr_state) {
		case LBA_UTILSTATE_UNK:
		case LBA_UTILSTATE_INIT:
		case LBA_UTILSTATE_RESET:
			ret = -EINVAL;
			break;
		case LBA_UTILSTATE_START:
		case LBA_UTILSTATE_STOP:
			break;
		default:
			return -EINVAL;
		}
		break;
	default:
		ret = -EINVAL;
	}

	if (ret < 0) {
		err("illegal transition from state \"%s\" to state \"%s\"\n", 
		    util_states_tab[curr_state].name, 
		    util_states_tab[new_state].name);
		return ret;
	}

	lba->util_state = new_state;
	switch (new_state) {
	case LBA_UTILSTATE_UNK:
		ret = -EINVAL;
		break;
	case LBA_UTILSTATE_INIT:
		ret = lba->ops->util_ops->util_init(lba->phys_addr);
		break;
	case LBA_UTILSTATE_RESET:
		ret = lba->ops->util_ops->util_reset(lba->phys_addr);
		break;
	case LBA_UTILSTATE_START:
		ret = lba->ops->util_ops->util_start(lba->phys_addr);
		break;
	case LBA_UTILSTATE_STOP:
		ret = lba->ops->util_ops->util_stop(lba->phys_addr);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static struct lba_ops *find_lba_ops(unsigned long function_id) {

	int i;

	for (i = 0; i < ARRAY_SIZE(lba_info_tab); i++) {
		if (function_id == lba_info_tab[i].function_id)
			return lba_info_tab[i].ops;
	}
	return NULL;
}

static acpi_status __init lba_probe(acpi_handle handle, u32 depth,
												void *context, void **ret)
{
	struct lba *lba;
	acpi_status status;
	unsigned long function_id;
	struct acpi_device *acpi_dev;
	struct acpi_pci_id pci_id;
	void *io_addr;
	u64 base_addr, length;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,27))
	unsigned long value;
#else
	unsigned long long value;
#endif
	
	status = hp_acpi_csr_space(handle, &base_addr, &length);
	if (ACPI_FAILURE(status)) 
		return AE_OK;

	/*
	 * Find the function id of this device.
	 */
	io_addr	  = ioremap(base_addr, 0);
	function_id = LBA_REG(io_addr, LBA_FUNC_ID);

	switch (function_id) {
	case APOLLO_FUNC_ID:
	case GEMINI_FUNC_ID:
	case MERCURY_FUNC_ID:
		break;
	default:
		info("Found LBA with unrecognized function ID 0x%lx.\n", 
		     function_id);
		return AE_OK;
		break;
	}
	
	if (acpi_bus_get_device(handle, &acpi_dev)) {
		err("Could not get ACPI device for device of type 0x%lx.\n",
		    function_id);
		return AE_OK;
	}

	if (ACPI_FAILURE(acpi_get_pci_id(handle, &pci_id))) {
		err("Could not get pci data for %s.\n", 
		    acpi_device_bid(acpi_dev));
		return AE_OK;
	}

	lba = kmalloc(sizeof(struct lba), GFP_KERNEL);
	memset(lba, 0, sizeof(struct lba));
	if (!lba) {
		err("Could not allocate memory for %s.\n",
		    acpi_device_bid(acpi_dev));
		return AE_OK;
	}

	lba->acpi_dev = acpi_dev;
	lba->phys_addr = io_addr;
	if (!(lba->ops = find_lba_ops(function_id))) {
		err("Could not find operations for function ID 0x%lu\n",
		    function_id);
		goto out_free;
	}

	status = acpi_evaluate_integer(handle, METHOD_NAME__SEG, NULL, &value);
	switch (status) {
	case AE_OK:
		lba->segment = (u16) value;
		break;
	case AE_NOT_FOUND:
		lba->segment = 0;
		break;
	default:
		lba->segment = 0xffff;
	}
	lba->util_state = LBA_UTILSTATE_UNK;

	if (register_hplba_device(lba, &pci_id)) {
		err("Could not register device for %s.\n",
		    acpi_device_bid(acpi_dev));
		goto out_free;
	}

	info("LBA %s found with ID 0x%lx at PCI %s\n",
	     acpi_device_bid(acpi_dev), function_id, get_device_id(lba));

	down(&lba_list_sem);
	list_add_tail(&lba->list, &lba_list);
	up(&lba_list_sem);

	return AE_OK;

out_free:
	kfree(lba);
	return AE_OK;
}

static int find_lbas(void)
{
	int i;

	for (i = 0; pnpids[i] != NULL; i++) {
		acpi_get_devices(pnpids[i], lba_probe, pnpids[i], NULL);
	}

	if (list_empty(&lba_list)) {
		err("No LBA's found on this platform.\n");
		return -1;
	}
	return 0;
}

static int __init hplba_init(void)
{
	int ret;

	info("%s %s\n", DRIVER_DESC, DRIVER_VERSION);

	if (register_hplba_class())
		 goto out_err;

	ret = find_lbas();
	if (ret == -1) 
		goto out_err;

	return 0;

out_err:
	return 1;
}

static void __exit hplba_exit(void)
{
	struct list_head *lh, *lh_sf;
	struct lba *lba;

	list_for_each_safe(lh, lh_sf, &lba_list) {
		lba = list_entry(lh, struct lba, list);
		list_del(&lba->list);
		unregister_hplba_device(lba);
		kfree(lba);
	}

	unregister_hplba_class();
}

module_init(hplba_init);
module_exit(hplba_exit);

