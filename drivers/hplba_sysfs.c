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

#include <linux/device.h>
#include <linux/acpi.h>
#include <linux/version.h>

#include "hplba.h"
#include "hplba_sysfs.h"

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
#  define SHOW_ARGS struct class_device *dev, char *buf
#  define STORE_ARGS struct class_device *dev, const char *buf, size_t count
#else
#  define class_device device
#  define class_device_attribute device_attribute
#  define class_dev_attrs dev_attrs
#  define class_id bus_id
#  define SHOW_ARGS struct device *dev, struct device_attribute *attr, \
		    char *buf
#  define STORE_ARGS struct device *dev, struct device_attribute *attr, \
		     const char *buf, size_t count
#endif

struct class_device_attribute lba_device_attrs[];

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
void lba_class_device_release(struct class_device *cd)
{
}
#else
void lba_device_release(struct device *dev)
{

}
#endif

void lba_class_release(struct class *class)
{
}

struct class lba_class = {
	.name = "hplba",
	.owner = THIS_MODULE,
	.class_dev_attrs = lba_device_attrs,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
	.release = lba_class_device_release,
#else
	.dev_release = lba_device_release,
#endif
	.class_release = lba_class_release,

};


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
int lba_cd_add(struct class_device *cd, struct class_interface *inf)
{
	return 0;
}

void lba_cd_remove(struct class_device *cd, struct class_interface *inf)
{
}

struct class_interface lba_class_inf = {
	.class = &lba_class,
	.add = lba_cd_add,
	.remove = lba_cd_remove,
}; 
#endif /* < linux-2.6.26 */

int register_hplba_class(void) 
{
	if (class_register(&lba_class)) {
		err("could not register class\n");
		goto out_class;
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
	if (class_interface_register(&lba_class_inf)) {
		err("Could not register class interface.\n");
		goto out_inf;
	}
#endif

	return 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
out_inf:
	class_interface_unregister(&lba_class_inf);
#endif
out_class:
	class_unregister(&lba_class);
	return 1;
}

void unregister_hplba_class(void) 
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
	class_interface_unregister(&lba_class_inf);
#endif
	class_unregister(&lba_class);
}

int register_hplba_device(struct lba *lba, struct acpi_pci_id *pci_id)
{
	int ret = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
	class_device_initialize(&lba->dev);
#else
	device_initialize(&lba->dev);
#endif

	lba->dev.class = &lba_class;
	snprintf(lba->dev.class_id, BUS_ID_SIZE, "%04x:%02x:%02x.%d",
		 pci_id->segment, pci_id->bus,
		 pci_id->device, pci_id->function);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
	ret = class_device_register(&lba->dev);
#else
	ret = device_add(&lba->dev);
#endif
	return ret;
}

void unregister_hplba_device(struct lba *lba)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
	class_device_unregister(&lba->dev);
#else
	device_unregister(&lba->dev);
#endif
}

char *get_device_id(struct lba *lba)
{
	return lba->dev.class_id;
}

static ssize_t function_id_show(SHOW_ARGS)	
{
	struct lba *lba = to_lba(dev);

	return sprintf(buf, "%#lx\n", LBA_REG(lba->phys_addr, LBA_FUNC_ID));
}

static ssize_t revision_show(SHOW_ARGS)	
{
	struct lba *lba = to_lba(dev);

	return sprintf(buf, "%s\n", lba->ops->revision(lba->phys_addr));
}

static ssize_t bus_op_mode_show(SHOW_ARGS)	
{
	struct lba *lba = to_lba(dev);

	return sprintf(buf, "%s\n", lba->ops->bus_op_mode(lba->phys_addr));
}

static ssize_t bus_speed_show(SHOW_ARGS)	
{
	struct lba *lba = to_lba(dev);

	return sprintf(buf, "%s\n", lba->ops->bus_speed(lba->phys_addr));
}

static ssize_t ropes_show(SHOW_ARGS)	
{
	struct lba *lba = to_lba(dev);

	return sprintf(buf, "%d\n", lba->ops->ropes(lba->phys_addr));
}

static ssize_t utilization_show(SHOW_ARGS)	
{
	struct lba *lba = to_lba(dev);
	uint64_t timer, cntr;

	lba->ops->utilization(lba->phys_addr, &timer, &cntr);

	return sprintf(buf, "%lu,%lu\n", timer, cntr);
}

static ssize_t utilization_store(STORE_ARGS)	
{
	struct lba *lba = to_lba(dev);
	int state;
	int ret;

	state = hplba_util_state_to_value(buf);
	if (state == LBA_UTILSTATE_UNK) {
		err("Invalid utilization command - %s\n", buf);
		return -EINVAL; 
	}
	if ((ret = set_util_state(lba, state)) < 0) {
		return ret; 
	}
	
	return count;
}

struct class_device_attribute lba_device_attrs[] = {
	__ATTR(function_id, S_IRUGO, function_id_show, NULL),
	__ATTR(revision, S_IRUGO, revision_show, NULL),
	__ATTR(bus_mode, S_IRUGO, bus_op_mode_show, NULL),
	__ATTR(bus_speed, S_IRUGO, bus_speed_show, NULL),
	__ATTR(ropes, S_IRUGO, ropes_show, NULL),
	__ATTR(utilization, S_IRUGO | S_IWUGO, 
	       utilization_show, utilization_store),
	__ATTR_NULL,
};

