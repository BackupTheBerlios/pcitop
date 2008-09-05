/*
 * HP IA-64 LBA utilization monitor
 *
 * Copyright (C) 2007 Hewlett-Packard Co
 * Copyright (C) 2007 Andrew Patterson <andrew.patterson@hp.com>
 * Copyright (C) 2007 Rick Jones <rick.jones2@hp.com>
 *
 * This program is free socimonftware; you can redistribute it and/or modify it
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


#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>

#include "hplba.h"
#include "pcitop.h"

#define PCITOP_VERSION "1.0.0"

const char LBA_DRIVER_NAME[] = "hplba";
const char *SLOT_DRIVER_NAMES[] = {
	"acpiphp",
	"pci_slot",
	NULL
};
const char SYSFS_CLASS_DIR[] = "/sys/class";
const char SYSFS_SLOTS_DIR[] = "/sys/bus/pci/slots";
const char SYSFS_DEV_DIR[] = "/sys/bus/pci/devices";

/* display constants */
#define COLUMN_SEP_WIDTH     2
#define HEADER_WIDTH	    14
#define COLUMN_DATA_WIDTH   (COLUMN_SEP_WIDTH + HEADER_WIDTH)
#define NUM_BANNER_ROWS	     7
#define SCREEN_ROWS	    24
#define UTIL_ROWS	    SCREEN_ROWS - NUM_BANNER_ROWS - 1

#define LINE_WIDTH(lbas)				\
	HEADER_WIDTH +					\
	((lbas) * COLUMN_DATA_WIDTH) +			\
	(((lbas) - 1) * COLUMN_SEP_WIDTH)

typedef struct {
	unsigned long	func_id;
	unsigned long	func_class;
} chipset_regs_t;

static pcitop_options_t options;
static struct lba_info *host_lba_list;
static int time_to_quit;
static char *prg_name;

struct bus_op_modes bus_op_modes_tab[]= {
	{LBA_UNK_TYPE,"unknown"},
	{LBA_AGP,"AGP"},
	{LBA_PCI,"PCI"},
	{LBA_PCI_X,"PCI-X mode 1"},
	{LBA_PCI_X_ECC,"mode 1 ECC"},
	{LBA_PCI_X2_ECC,"PCI-X mode 2 ECC"},
	{LBA_PCIE,"PCIe"},
};

struct bus_speeds bus_speeds_tab[] = {
	{LBA_UNK_SPEED, "unknown"},
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
};

struct util_states util_states_tab[] = {
	{LBA_UTILSTATE_UNK, "unknown"},
	{LBA_UTILSTATE_INIT, "init"},
	{LBA_UTILSTATE_RESET, "reset"},
	{LBA_UTILSTATE_START, "start"},
	{LBA_UTILSTATE_STOP, "stop"},
};

void note(char *fmt, ...);
void error(char *fmt, ...);
void fatal_error(char *fmt, ...);
void vbprintf(char *fmt, ...);
void usage(const char *prg_name);
void mask_signals(void);
void unmask_signals(void);
int driver_loaded(const char *name);
int drivers_loaded(void);
int hplba_init(struct lba_info *lba);
int hplba_reset(struct lba_info *lba);
int hplba_start(struct lba_info *lba);
int hplba_stop(struct lba_info *lba);
int hplba_collect(struct lba_info *lba);
struct lba_info *register_lba(const char *name);
int find_interface_in_dir(const char *dirname, const char *ifname);
int find_interface(const char *ifname, char lspci[13]);
int slot_add(struct lba_info *lba, const char *name);
int itanium_parse_slot(const char *name,
		       char *cabinet, 
		       char *chassis,
		       char *bay,
		       char *slot);
int find_lba_location_info(struct lba_info *lba);
int pcicmp(char *pciaddr, unsigned short domain, unsigned int bus_id);
int name_to_pci_id(const char *name, struct pci_id *id);
void filter_match_and(struct lba_info *lba);
void filter_match_or(struct lba_info *lba);
int read_attribute(const char *name, char *contents);
int read_lba_attribute(struct lba_info *lba, const char *attribute,
		       char *contents);
int write_attribute(const char *path, const char *contents);
int write_lba_attribute(struct lba_info *lba, const char *attribute, 
			const char *contents);
struct lba_info *init_lba(const char *name);
void filter_lbas(void);
int bridge_in_use(struct lba_info *lba);
int init_lbas(void);
void show_all_root_info(void);
char *build_banner(unsigned int num_lbas);
void measure_utilization(void);
void sigint_handler(int n);
void setup_signal(void);

void note(char *fmt, ...) 
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}

void error(char *fmt, ...) 
{
	va_list ap;

	fprintf(stderr, "%s: ", prg_name);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

void fatal_error(char *fmt, ...) 
{
	va_list ap;

	fprintf(stderr, "%s: ", prg_name);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(1);
	/* NOT REACHED */
}

void vbprintf(char *fmt, ...)
{
	va_list ap;

	if (options.opt_verbose == 0)
		return;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

static struct option pcitop_option_list[]={
	{ "help", 0, 0, 1 },
	{ "version", 0, 0, 2 },
	{ "timeout", 1, 0, 3 },
	{ "info", 0, 0, 4 },
	{ "verbose", 0, &options.opt_verbose, 1 },
	/* { "bus", 1, &options.opt_match_bus, -1 }, */
	{ "debug", 0, &options.opt_debug, 1 },
	{ 0, 0, 0, 0}
};

void usage(const char *prg_name)
{
	printf( "usage: %s [OPTIONS]\n", prg_name);
	printf( "-h, -?, --help\t\tthis help message\n"
		"-a\t\t\tlogically 'and' the display filters\n"
		"-A chassis\t\tdisplay buses in chAssis\n"
		"-B bay\t\t\tdisplay buses in Bay\n"
		"-b bus\t\t\tdisplay buses matching bus\n"
		"-C cabinet\t\tdisplay buses in Cabinet\n"
		"-D domain\t\tdisplay buses in PCI Domain\n"
		"-o\t\t\tlogically 'or' the display filters\n"
		"-P lspci \t\tdisplay bus matching 'lspci-esque' pci address\n"
		"-S slot\t\t\tdisplay buses matching slot number\n"
		"-v,--verbose\t\tverbose mode\n"
		"-V,--version\t\tprint version\n"
		"-i,--info\t\tprint bus information only\n"
		"-t,--timeout\t\tset print interval in seconds [default 1s]\n"
		"-c num\t\t\tnumber of samples [default 60]\n");
}

void mask_signals(void)
{
	sigset_t my_set;

	sigemptyset(&my_set);
	sigaddset(&my_set, SIGINT);
	sigaddset(&my_set, SIGALRM);
	sigprocmask(SIG_BLOCK, &my_set, NULL);
}

void unmask_signals(void)
{
	sigset_t my_set;

	sigemptyset(&my_set);
	sigaddset(&my_set, SIGINT);
	sigprocmask(SIG_UNBLOCK, &my_set, NULL);
}

int driver_loaded(const char *name)
{
	char modpath[PATH_MAX];
	struct stat statbuf;

	snprintf(modpath, PATH_MAX-1,"%s%s", "/sys/module/", name);
	if (stat(modpath, &statbuf) < 0)
		return 0;
	return 1;
}

int drivers_loaded(void)
{
	int i, found;

	if (!driver_loaded(LBA_DRIVER_NAME))
	{
		error("driver %s not loaded\n", LBA_DRIVER_NAME);
		return 0;
	}

	found = 0;
	for (i = 0; SLOT_DRIVER_NAMES[i]; i++) {
		if (driver_loaded(SLOT_DRIVER_NAMES[i])) {
			found = 1;
			break;
		}
	}

	if (!found) {
		char drivers[80];
		char *p = drivers;

		for (i = 0; SLOT_DRIVER_NAMES[i]; i++) {
			if (SLOT_DRIVER_NAMES[i + 1])
				p += sprintf(p, "\"%s\" or ", 
					     SLOT_DRIVER_NAMES[i]);
			else
				p += sprintf(p, "\"%s\"", SLOT_DRIVER_NAMES[i]);
		}
		error("cannot find one of the following drivers %s -- at least one of these drivers must be loaded\n", drivers);
		return 0;
	}

	return 1;
}

/* Generic set of LBA operations for use with hplba driver. */
int hplba_init(struct lba_info *lba)
{
	return write_lba_attribute(lba, "utilization", "init");
}

int hplba_reset(struct lba_info *lba)
{
	return write_lba_attribute(lba, "utilization", "reset");
}

int hplba_start(struct lba_info *lba)
{
	return write_lba_attribute(lba, "utilization", "start");
}

int hplba_stop(struct lba_info *lba)
{
	return write_lba_attribute(lba, "utilization", "stop");
}

int hplba_collect(struct lba_info *lba)
{
	static char *utilization = 0;

	int retval = 0;
	unsigned long timer;
	unsigned long counter;
	int page_size = getpagesize();

	if (!utilization) {
		utilization = malloc(page_size);
		if (!utilization) {
			error("%s cannot allocate memory\n");
			goto out;
		}
	}

	memset(utilization, 0, page_size);
	read_lba_attribute(lba, "utilization", utilization);
	if (sscanf(utilization, "%lu,%lu", &timer, &counter) != 2) {
		error("cannot parse utilization counter for %s\n", lba->name);
		retval = 1;
		goto out;
	}

	lba->timer = timer;
	lba->counter = counter;

out:
	return retval;
}

struct lba_ops hplba_ops = {
	hplba_init,
	hplba_reset,
	hplba_start,
	hplba_stop,
	hplba_collect,
};

struct lba_info *register_lba(const char *name)
{
	struct lba_info *new_lba, **tmp;
	new_lba = calloc(1, sizeof(*new_lba));
	if (!new_lba)
		fatal_error("cannot allocate memory for host local bus "
			    "adapter\n");
	new_lba->name = name;
	for (tmp = &host_lba_list; 
	     *tmp; 
	     tmp = &(*tmp)->next)
		;
	*tmp = new_lba;
	return new_lba;
}

int find_interface_in_dir(const char *dirname, const char *ifname)
{
	DIR *device_dir;
	struct dirent *device_dp;

	device_dir = opendir(dirname);
	if (!device_dir)
		return 0;

	while ((device_dp = readdir(device_dir)) != NULL) {
		char netname[8];
		if ((sscanf(device_dp->d_name,"net:%s",netname) == 1) &&
		    (strcmp(netname,ifname) == 0)) {
			closedir(device_dir); 
			return 1;
		}
	}
	closedir(device_dir);
	return 0;
}

int find_interface(const char *ifname, char lspci[13])
{
	DIR *devices_dir;
	struct dirent *devices_dp;
	DIR *pci_dir;
	struct dirent *pci_dp;
	char dirname[PATH_MAX];

	devices_dir = opendir("/sys/devices");
	if (!devices_dir) {
		/* just return a -1 which means no try to match */
		return -1;
	}
	while ((devices_dp = readdir(devices_dir)) != NULL) {
		int domain, bus;
		if (sscanf(devices_dp->d_name,"pci%d:%x",&domain,&bus) == 2) {
			snprintf(dirname,PATH_MAX-1,"%s/%s",
				  "/sys/devices",devices_dp->d_name);
			pci_dir = opendir(dirname);
			if (pci_dir == NULL)
				continue;
			while ((pci_dp = readdir(pci_dir)) != NULL) {
				char pciname[PATH_MAX];
				snprintf(pciname,PATH_MAX -1,"%s/%s",
					 dirname,pci_dp->d_name);
				if (find_interface_in_dir(pciname,ifname)) {
					closedir(pci_dir);
					closedir(devices_dir);
					sprintf(lspci,"%04x:%02x:%02x,%01x",
						domain,bus,0,0);
					return 1;
				}	
			}
			closedir(pci_dir);
		}
	}
	closedir(devices_dir);
	return -1;
}

int add_slot(struct lba_info *lba, const char *name)
{

	struct slot *new_slot, **tmp_slot;
	new_slot = calloc(1, sizeof(struct slot));
	if (!new_slot)
		fatal_error("cannot allocate memory for slot %s\n", name);
	new_slot->name = strndup(name, SLOT_NAME_SIZE);
	for (tmp_slot = &lba->slot;
	     *tmp_slot; 
	     tmp_slot = &(*tmp_slot)->next){
	}
	*tmp_slot = new_slot;

	return 0;
}



/* 
 * On Itanium systems, slots are found in /sys/bus/pci/slots
 * if the acpiphp or pci_slots driver is loaded.  Slots can be
 * in cabinets, chassis, and bays. The format for the slot
 * entry is:
 *
 *  WXYZZ where:
 *
 * W = cabinet
 * X = bay
 * Y = chassis
 * ZZ = slot number
 *
 * If there only one cabinet, chassis, bay then these fields are
 * should be omitted.  We set them to zero instead as you need per machine
 * info to determine the presence of or absence of them.
 */
int itanium_parse_slot(const char *name,
		       char *cabinet, 
		       char *chassis,
		       char *bay,
		       char *slot)
{
	strcpy(cabinet, "0");
	strcpy(bay, "0");
	strcpy(chassis, "0");
	strcpy(slot, "0");
	int len = strlen(name);

	if (len == 5) {
		cabinet[0] = name[0];
		name++;
	}
	if (len >= 4) {
		bay[0] = name[0];
		name++;
	}
	if (len >= 3) {
		chassis[0] =  name[0];
		name++;
	}
	if (len >= 2) {
		if (name[0] != '0') {
			strncpy(slot, name, 2);
			return 0;
		}
		name++;
	}			
	if (len >= 1) {
		strncpy(slot, name, 1);
	}

	return 0;
}

int find_lba_location_info(struct lba_info *lba)
{
	int retval = 1;
	unsigned domain, pci_addr, bus_id;
	int len;
	DIR *slots_dir;
	struct dirent *slots_dp;
	char address_path[PATH_MAX];
	char *address;
	char cabinet[ITANIUM_CABINET_NAME_SIZE];
	char chassis[ITANIUM_CHASSIS_NAME_SIZE];
	char bay[ITANIUM_BAY_NAME_SIZE];
	char slot[ITANIUM_SLOT_NAME_SIZE];

	/**** TODO -- need match slot to root bridge ****/
	
	slots_dir = opendir(SYSFS_SLOTS_DIR);
	if (!slots_dir) {
		error("could not open %s\n", SYSFS_SLOTS_DIR);
		goto out;
	}

	address = calloc(1, getpagesize());
	if (!address) {
		error("%s cannot allocate memory\n");
		goto dir;
	}
	while ((slots_dp = readdir(slots_dir)) != NULL) {
		if (strlen(slots_dp->d_name) == 0 ||
		    strcmp(slots_dp->d_name,".") == 0 ||
		    strcmp(slots_dp->d_name,"..") == 0)
			continue;
		snprintf(address_path, PATH_MAX, "%s/%s/address", 
			 SYSFS_SLOTS_DIR, slots_dp->d_name);
		read_attribute(address_path, address);
		len = sscanf(address, "%4x:%2x:%2x",
			     &domain, &bus_id, &pci_addr);
		if (len != 3) {
			error("could not parse %s\n", address_path);
			goto mem;
		}
		if ((lba->pci_id.pci_busid == bus_id) &&
		    (lba->pci_id.pci_domain == domain)) {
			itanium_parse_slot(slots_dp->d_name,
					   cabinet,
					   chassis,
					   bay,
					   slot);
			if (lba->cabinet[0] == '\0')
				lba->cabinet[0] = cabinet[0];
			if (lba->bay[0] == '\0')
				lba->bay[0] = bay[0];
			if (lba->chassis[0] == '\0')
				lba->chassis[0] = chassis[0];
			add_slot(lba, slot);
		}
	}

mem:
	free(address);
dir:
	closedir(slots_dir);
out:
	return retval;
}

int pcicmp(char *pciaddr, unsigned short domain, unsigned int bus_id)
{
	int len,ret;
	unsigned int pci_domain,pci_busid,pci_addr,pci_func;

	len = strlen(pciaddr);
	switch (len) {
	case 7:
		ret = sscanf(pciaddr,"%02x:%02x.%01x",
			     &pci_busid,
			     &pci_addr,
			     &pci_func); 
		pci_domain = 0;
		break;
	case 12:
		ret = sscanf(pciaddr,"%04x:%02x:%02x.%01x",
			     &pci_domain,
			     &pci_busid,
			     &pci_addr,
			     &pci_func);
		break;
	default:
		return -1;
	}

	if ((pci_domain == domain) && (pci_busid == bus_id))
		return 0;
	else
		return 1;
}

int name_to_pci_id(const char *name, struct pci_id *id)
{
	int len;
	int ret = 0;

	len = strlen(name);
	switch (len) {
	case 7:
		ret = sscanf(name, "%02x:%02x.%01x",
			     &id->pci_busid,
			     &id->pci_addr,
			     &id->pci_func);
		if (ret != 4)
			ret = -1;
		id->pci_domain = 0;
		break;
	case 12:
		ret = sscanf(name, "%04x:%02x:%02x.%01x",
			     &id->pci_domain,
			     &id->pci_busid,
			     &id->pci_addr,
			     &id->pci_func);
		if (ret != 5)
			ret = -1;
		break;
	default:
		ret = -1;
	}

	return ret;
}

int match_slot(struct lba_info *lba, const char *name)
{
	struct slot *slot = lba->slot;

	while (slot) {
		if (strncmp(slot->name, name, ITANIUM_SLOT_NAME_SIZE) == 0)
			return 1;
		slot = slot->next;
	}
	return 0;
}

void filter_match_and(struct lba_info *lba)
{
	if ((options.opt_match_domain >= 0) &&
	    ((int)lba->pci_id.pci_domain != options.opt_match_domain)) {
		lba->display = 0;
		return;
	}
	if ((options.opt_match_bus >= 0) &&
	    ((int)lba->pci_id.pci_busid != options.opt_match_bus)) {
		lba->display = 0;
		return;
	}
	if ((options.opt_match_lspci) &&
	    (pcicmp(options.opt_match_lspci, 
		    lba->pci_id.pci_domain, 
		    lba->pci_id.pci_busid) != 0)) {
		lba->display = 0;
		return;
	} 
	if ((options.opt_match_cabinet) &&
	    (strcmp(options.opt_match_cabinet,lba->cabinet) != 0)) {
		lba->display = 0;
		return;
	}
	if ((options.opt_match_bay) &&
	    (strcmp(options.opt_match_bay,lba->bay) != 0)) {
		lba->display = 0;
		return;
	}
	if ((options.opt_match_chassis) &&
	    (strcmp(options.opt_match_chassis,lba->chassis) != 0)) {
		lba->display = 0;
		return;
	}
	if (options.opt_match_slot && 
	    !match_slot(lba, options.opt_match_slot)) {
		lba->display = 0;
		return;
	}

	options.num_lba_display++;
	lba->display = 1;
}

void filter_match_or(struct lba_info *lba)
{
	lba->display = 0;

	/* looks ungainly, but I like it better than a huge single conditional */
	if ((options.opt_match_domain >= 0) &&
	    ((int)lba->pci_id.pci_domain == options.opt_match_domain)){
		lba->display = 1;
		options.num_lba_display++;
		return;
	}
	if ((options.opt_match_bus >= 0) &&
	    ((int)lba->pci_id.pci_busid == options.opt_match_bus)) {
		lba->display = 1;
		options.num_lba_display++;
		return;
	}
	if ((options.opt_match_lspci) &&
	    (pcicmp(options.opt_match_lspci, 
		    lba->pci_id.pci_domain, 
		    lba->pci_id.pci_busid) == 0)) {
		lba->display = 1;
		options.num_lba_display++;
		return;
	} 
	if ((options.opt_match_cabinet) &&
	    (strcmp(options.opt_match_cabinet, lba->cabinet) == 0)) {
		lba->display = 1;
		options.num_lba_display++;
		return;
	}
	if ((options.opt_match_bay) &&
	    (strcmp(options.opt_match_bay, lba->bay) == 0)) {
		lba->display = 1;
		options.num_lba_display++;
		return;
	}
	if ((options.opt_match_chassis) &&
	    (strcmp(options.opt_match_chassis, lba->chassis) == 0)) {
		lba->display = 1;
		options.num_lba_display++;
		return;
	}
	if (options.opt_match_slot && match_slot(lba, options.opt_match_slot)) {
		lba->display = 1;
		options.num_lba_display++;
		return;
	}
}

int read_attribute(const char *path, char *contents)
{
	static char *buf = 0;

	int fd;
	int retval = 1;
	ssize_t amt_read;
	int page_size = getpagesize();
	
	if (!buf) {
		buf = malloc(page_size);
		if (!buf) {
			error("%s cannot allocate memory\n");
			goto out;
		}
	}
		     
	fd = open(path, O_RDONLY);
	if (fd < 0) {
		error("cannot open %s -- %s\n", path, strerror(errno));
		goto out;
	}

	memset(buf, 0, page_size);
	amt_read = read(fd, buf, page_size);
	if (amt_read < 0) {
		error("cannot read attribute %s -- %s\n", path,
		      strerror(errno));
		goto fd;
	}
	if (strlen(buf) > 0)
		buf[strlen(buf) - 1] = '\0';
	strncpy(contents, buf, page_size);
	retval = 0;

fd:
	close(fd);

out:
	return retval;
}


int read_lba_attribute(struct lba_info *lba, const char *attribute, 
		       char *contents)
{
	char path[PATH_MAX];
	
	snprintf(path, PATH_MAX, "%s/%s/%s/%s", 
		 SYSFS_CLASS_DIR, LBA_DRIVER_NAME, lba->name, attribute);
	return read_attribute(path, contents);
}

int write_attribute(const char *path, const char *contents)
{
	int fd;
	int retval = 1;
	ssize_t amt_written;
	
	fd = open(path, O_WRONLY);
	if (fd < 0) {
		error("cannot open %s -- %s\n", path, strerror(errno));
		goto out;
	}

	amt_written = write(fd, contents, strlen(contents) + 1);
	if (amt_written < 0) {
		error("cannot write attribute %s -- %s\n", path,
		      strerror(errno));
		goto fd;
	}
	retval = 0;

fd:
	close(fd);

out:
	return retval;
}

int write_lba_attribute(struct lba_info *lba, const char *attribute, 
			const char *contents)
{
	char path[PATH_MAX];
	
	snprintf(path, PATH_MAX, "%s/%s/%s/%s", 
		 SYSFS_CLASS_DIR, LBA_DRIVER_NAME, lba->name, attribute);
	return write_attribute(path, contents);
}

int bridge_in_use(struct lba_info *lba)
{
	DIR *dev_dir;
	struct dirent *dev_dp;
	unsigned int domain, bus_id;
	int len;
	int found = 0;

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
		len = sscanf(dev_dp->d_name, "%4x:%2x", &domain, &bus_id);
		if (len != 2) {
			error("could not parse %s\n", dev_dp->d_name);
		}
		if ((lba->pci_id.pci_busid == bus_id) &&
		    (lba->pci_id.pci_domain == domain)) {
			found = 1;
			break;
		}
	}
	closedir(dev_dir);

out:
	return found;
}

struct lba_info *init_lba(const char *name)
{
	struct lba_info *lba = NULL;
	char *attrib;
	char *lba_name;
	struct pci_id pci_id;

	lba_name = strndup(name, PATH_MAX);
	if (!lba_name) {
		error("%s cannot allocate memory\n");
		goto out;
	}

	lba = register_lba(lba_name);
	if (!lba) {
		error("cannot register lba %s\n", name);
		goto out;
	}

	name_to_pci_id(name, &pci_id);
	memcpy(&lba->pci_id, &pci_id, sizeof(struct pci_id));
	
	lba->ops = &hplba_ops;
	lba->display = 1;

	attrib = calloc(1, getpagesize());
	if (!attrib) {
		error("%s cannot allocate memory\n");
		goto out;
	}

	read_lba_attribute(lba, "bus_mode", attrib);
	lba->bus_type = 
		hplba_bus_op_mode_to_str(hplba_bus_op_mode_to_value(attrib)); 
	read_lba_attribute(lba, "bus_speed", attrib);
	lba->bus_speed = 
		hplba_bus_speed_to_str(hplba_bus_speed_to_value(attrib));
	read_lba_attribute(lba, "function_id", attrib);
	sscanf(attrib, "0x%x", &lba->id);
	read_lba_attribute(lba, "revision", attrib);
	strncpy(lba->revision, attrib, REV_LEN);
	read_lba_attribute(lba, "ropes", attrib);
	lba->ropes = atoi(attrib);
	lba->in_use = bridge_in_use(lba);
	options.num_lba++;

	
out:
	return lba;
}

void filter_lbas(void)
{
	struct lba_info *lba = host_lba_list;
	
	while (lba) {
		if (options.opt_match_and)
			filter_match_and(lba);
		else
			filter_match_or(lba);
		lba = lba->next;
	}
}

int init_lbas(void)
{
	DIR *dir;
	
	char path[PATH_MAX];
	int retval = 1;
	struct lba_info *lba = NULL;

	/* Traverse module dir looking for lba entries */
	snprintf(path, PATH_MAX, "%s/%s", SYSFS_CLASS_DIR, LBA_DRIVER_NAME);
	dir = opendir(path);
	if (!dir) {
		error("cannot find hplba sysfs entry %s -- %s\n",
		      path, strerror(errno));
		goto out;
	}
	for (;;) {
		struct dirent *dentry = readdir(dir);
		if (!dentry)
			break;
		if (strcmp(dentry->d_name, ".") == 0 ||
		    strcmp(dentry->d_name, "..") == 0)
			continue;
		lba = init_lba(dentry->d_name);
		find_lba_location_info(lba);
	}
	closedir(dir);
	retval = 0;
out:
	return retval;
}

void show_all_root_info(void)
{
	struct lba_info *lba;
	struct slot *slot;
	for (lba = host_lba_list; lba; lba = lba->next) {
		printf("%-14s %s/%s lba id = %#x rev=%s in use=%s ropes=%d "
		       "cabinet %s bay %s chassis %s slots ",
		       lba->name, lba->bus_type, lba->bus_speed,
		       lba->id, lba->revision, 
		       (lba->in_use ? "y" : "n"),
		       lba->ropes,
		       (lba->cabinet[0] == '\0') ? "n/a" : lba->cabinet,
		       (lba->bay[0] == '\0') ? "n/a" : lba->bay,
		       (lba->chassis[0] == '\0') ? "n/a" : lba->chassis);
		slot = lba->slot;
		if (slot)
			do {
				printf("%s", slot->name);
				if (slot->next)
					printf(",");
				slot = slot->next;
			} while (slot);
		else
			printf("n/a");

		printf("\n");
	}
	printf("Found %lu PCI/PCI-X/PCIe root bridges\n", options.num_lba);
}

char *build_banner(unsigned int num_lbas)
{
	struct lba_info *lba;
	unsigned int i;
	int col_width; 
	char *banner, *p;

	banner = malloc(((LINE_WIDTH(num_lbas) + 1) * NUM_BANNER_ROWS) + 1);
	if (!banner) {
		error("could not allocate memory for banner\n");
		return NULL;
	}
	p = banner;
	for(i = 0; i < LINE_WIDTH(num_lbas); i++) {
		*p++ = '-';
	}
	*p++ = '\n';
	p += sprintf(p, "%*s", -HEADER_WIDTH, "bridge");
	for (lba = host_lba_list; lba; lba = lba->next) {
		if (lba->display) {
			col_width = COLUMN_DATA_WIDTH;
			if (lba->next)
				col_width += COLUMN_SEP_WIDTH;
			p += sprintf(p, "%*s", -col_width, lba->name);
		}
	}
	*p++ = '\n';
	p += sprintf(p, "%*s", -HEADER_WIDTH, "type");
	for (lba = host_lba_list; lba; lba = lba->next) {
		if (lba->display) {
			col_width = COLUMN_DATA_WIDTH;
			if (lba->next)
				col_width += COLUMN_SEP_WIDTH;
			p += sprintf(p, "%*s", -col_width, lba->bus_type);
		}
	}
	*p++ = '\n';
	p += sprintf(p, "%*s", -HEADER_WIDTH, "speed");
	for (lba = host_lba_list; lba; lba = lba->next) {
		if (lba->display) {
			col_width = COLUMN_DATA_WIDTH;
			if (lba->next)
				col_width += COLUMN_SEP_WIDTH;
			p += sprintf(p, "%*s", -col_width, lba->bus_speed);
		}
	}
	*p++ = '\n';
	p += sprintf(p, "%*s", -HEADER_WIDTH, "slots");
	for (lba = host_lba_list; lba; lba = lba->next) {
		if (lba->display) {
			struct slot *slot = lba->slot;
			int cnt = 0, tot_cnt = 0;
			if (slot) {
				while (slot) {
					cnt = sprintf(p, "%s", slot->name);
					p += cnt;
					tot_cnt += cnt;
					if (slot->next) {
						cnt = sprintf(p, "%s", ", ");
						p += cnt;
						tot_cnt += cnt;
					}
					slot = slot->next;
				}
				p += sprintf(p, "%*s", 
					     COLUMN_DATA_WIDTH - tot_cnt,
					     " ");
			}
			else {
				p += sprintf(p, "%*s", 
					     -COLUMN_DATA_WIDTH, "n/a");
			}
			if (lba->next)
				p += sprintf(p, "%*s", 
					     COLUMN_SEP_WIDTH, " ");
		}
	}
	*p++ = '\n';
	p += sprintf(p, "%*s", -HEADER_WIDTH, "ropes");
	for (lba = host_lba_list; lba; lba = lba->next) {
		if (lba->display) {
			p += sprintf(p, "%*d", 
				     -COLUMN_DATA_WIDTH,
				     lba->ropes);
			if (lba->next)
				p += sprintf(p, "%*s", COLUMN_SEP_WIDTH, " ");
		}
	}
	*p++ = '\n';
	for (i = 0; i < LINE_WIDTH(num_lbas); i++) {
		*p++ = '-';
	}

	return banner;
}

void measure_utilization(void)
{
	unsigned int num_lba_display;
	unsigned int num_lines = UTIL_ROWS;
	int col_width;
	unsigned long nsamples;
	unsigned long timer, counter;
	int timeout;
	double util;
	char *banner, *line_results, *p;
	struct lba_info *lba;

	num_lba_display = options.num_lba_display;
	banner = build_banner(num_lba_display);
	if (!banner)
		exit(1);
	timeout = options.timeout;

	line_results = malloc(LINE_WIDTH(num_lba_display) + 1); 
	if (!line_results) 
		fatal_error("could not allocate memory for line_results\n");

	/* program counters */
	for (lba = host_lba_list; lba; lba = lba->next) {
		if (lba->display)
			lba->ops->init(lba);
	}

	for (lba = host_lba_list; lba; lba = lba->next) {
		if (lba->display)
			lba->ops->reset(lba);
	}

	for (nsamples = options.nsamples; 
	     time_to_quit == 0 && nsamples;
	     num_lines++, nsamples--) {

		for (lba = host_lba_list; lba; lba = lba->next) {
			if (lba->display)
				lba->ops->start(lba);
		}
		unmask_signals();

		sleep(timeout);

		mask_signals();

		for (lba = host_lba_list; lba; lba = lba->next) {
			if (lba->display)
				lba->ops->stop(lba);
		}

		for (lba = host_lba_list; lba; lba = lba->next) {
			if (lba->display)
				lba->ops->collect(lba);
		}

		if (num_lines == UTIL_ROWS) {
			puts(banner);
			num_lines = 0;
		}
		/*
		 * account for header
		 */
		p = line_results;
		p += sprintf(p, "%*s", -HEADER_WIDTH, " ");

		/* 
		 * process new counts
		 */
		for (lba = host_lba_list; lba; lba = lba->next) {
			if (lba->display) {
				timer = lba->timer; 
				counter = lba->counter; 
				/* Bug in lba counters always shows 100%
				   utilization when no devices behind it. */
				if (lba->in_use)
					util = (counter - lba->prev_counter)*100.0/(timer - lba->prev_timer);
				else
					util = 0.0;
				lba->prev_timer = timer;
				lba->prev_counter = counter;
				col_width = COLUMN_DATA_WIDTH;
				if (lba->next)
					col_width += COLUMN_SEP_WIDTH;
				p += sprintf(p, "%06.2f%%%*s", 
					     util, col_width - 7, " ");
			}
		}
		puts(line_results);

	}
	unmask_signals();

	if (nsamples) 
		printf("interrupted after %lu samples -- "
		       "printing average results...\n",
		       options.nsamples - nsamples);
	puts(banner);
	p = line_results;
	p += sprintf(p, "%*s", -HEADER_WIDTH, " ");
	for (lba = host_lba_list; lba; lba = lba->next) {
		if (lba->display) {
			if (lba->in_use)
				util = (lba->counter * 100.0)/lba->timer;
			else
				util = 0.0;
			col_width = COLUMN_DATA_WIDTH;
			if (lba->next)
				col_width += COLUMN_SEP_WIDTH;
			p += sprintf(p, "%06.2f%%%*s", 
				     util, col_width - 7, " ");
		}
	}
	puts(line_results);
}

void sigint_handler(int n)
{
	n = n; /* suppress "unused parameter" compiler warning */
	time_to_quit = 1;
}

void setup_signal(void)
{
	struct sigaction act;

	act.sa_handler = sigint_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGINT, &act, NULL);
}

int main(int argc, char **argv)
{
	int c;
	char my_lspci[13];

	prg_name = basename(argv[0]);

	options.num_lba = 0;
	options.num_lba_display = 0;
	options.opt_match_bus = -1; 
	options.opt_match_domain = -1;
	options.opt_match_and = 1;
	options.opt_match_or = 0;
	options.opt_match_cabinet = NULL;
	options.opt_match_bay = NULL;
	options.opt_match_chassis = NULL;
	options.opt_match_slot = NULL;
	
	while ((c=getopt_long(argc, argv,
			      "aA:b:B:C:D:hI:oP:S:vdVit:c:?", 
			      pcitop_option_list, 0)) != -1) {
		switch(c) {
			case   0: continue; /* fast path for options */

			case 'A': options.opt_match_chassis = strdup(optarg);
				  break;
			case 'a': options.opt_match_and = 1;
				  options.opt_match_or = 0;
				  break;
			case 'v': options.opt_verbose = 1;
				  break;
			case 'B': options.opt_match_bay = strdup(optarg);
				  break;
			case 'b': options.opt_match_bus = strtol(optarg,NULL,0);
				  break;
			case 'C': options.opt_match_cabinet = strdup(optarg);
				  break;
			case 'c':
				  options.nsamples = strtoul(optarg, NULL, 10);
				  break;
			case 'D':
				  options.opt_match_domain = strtol(optarg,NULL,0);
				  break;
			case 'd': options.opt_debug = 1;
				  break;
			case 'I': if (find_interface(optarg,my_lspci) == 1)
					options.opt_match_lspci = strdup(my_lspci);;
				  break;
			case 'o': options.opt_match_or = 1;
				  options.opt_match_and = 0;
				  break;
			case 'P': options.opt_match_lspci = strdup(optarg);
				  break;
			case 'S': options.opt_match_slot = strdup(optarg);
				  break;
			case 'h':
			case '?':
			case 1: usage(prg_name); 
				exit(0);
			case 3:
			case 't':
				options.timeout = atoi(optarg);
				break;
			case 4:
			case 'i':
				options.opt_show_info = 1;
				break;
			case 'V': 
			case   2:
				printf("%s version " PCITOP_VERSION " Date: "
				       __DATE__ "\n"
				       "Copyright (C) 2004-2008 "
				       "Hewlett-Packard Co.\n", prg_name);
				exit(0);
			default:
				fatal_error("");
		}
	}

	if (geteuid() != 0)
		fatal_error("you must be root to run this program\n");

	if (!drivers_loaded())
		fatal_error("one or more required drivers are not loaded - "
			    "try insmod or modprobe\n");

	if (options.nsamples == 0) 
		options.nsamples = 60;

	setup_signal();

	if (init_lbas())
		exit(1);

	if (options.timeout == 0) 
		options.timeout = 1;

	if (options.opt_show_info) {
		show_all_root_info();
		return 0;
	}

	filter_lbas();

	if (options.num_lba == 0)
		fatal_error("this machine does not have suitable I/O "
			    "controllers\n");

	if (options.num_lba_display == 0)
		fatal_error("no I/O controllers matched filter\n");

	measure_utilization();

	return 0;
}
