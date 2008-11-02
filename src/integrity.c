/*
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
#include <errno.h>

#include "integrity.h"
#include "util.h"


int integrity_init_sys_info(struct integrity_sys_info *sys_info)
{
	memset(sys_info, 0, sizeof(struct integrity_sys_info));
	list_head_init(&sys_info->cabinet_list);
	list_head_init(&sys_info->bay_list);
	list_head_init(&sys_info->chassis_list);

	return 0;
}

int integrity_update_lba_info(struct integrity_lba_info *info, int slot_num) 
{
	if (slot_num != INTEGRITY_NONE)
		info->num_slots++;

	return 0;
}

int integrity_update_sys_info(struct integrity_lba_info *lba_info,
			      struct integrity_sys_info *sys_info)
{
	int found;
	struct cabinet_info *cabinet, *new_cabinet;
	struct bay_info *bay, *new_bay;
	struct chassis_info *chassis, *new_chassis;

	found = 0;
	list_for_each(&sys_info->cabinet_list, cabinet, list) {
		if (lba_info->cabinet == cabinet->number) {
			found = 1;
			break;
		}
	}
	if (!found) {
		new_cabinet = calloc(1, sizeof(struct cabinet_info));
		if (!new_cabinet) {
			error("out of memory\n");
			return -ENOMEM;
		}
		new_cabinet->number = lba_info->cabinet;
		sys_info->num_cabinets++;
		list_add_tail(&sys_info->cabinet_list,
			      &new_cabinet->list);
	}

	found = 0;
	list_for_each(&sys_info->bay_list, bay, list) {
		if (lba_info->bay == bay->number) {
			found = 1;
			break;
		}
	}
	if (!found) {
		new_bay = calloc(1, sizeof(struct bay_info));
		if (!new_bay) {
			error("out of memory\n");
			return -ENOMEM;
		}
		new_bay->number = lba_info->bay;
		sys_info->num_bays++;
		list_add_tail(&sys_info->bay_list,
			      &new_bay->list);
	}

	found = 0;
	list_for_each(&sys_info->chassis_list, chassis, list) {
		if (lba_info->chassis == chassis->number) {
			found = 1;
			break;
		}
	}
	if (!found) {
		new_chassis = calloc(1, sizeof(struct chassis_info));
		if (!new_chassis) {
			error("out of memory\n");
			return -ENOMEM;
		}
		new_chassis->number = lba_info->chassis;
		sys_info->num_chassis++;
		list_add_tail(&sys_info->chassis_list,
			      &new_chassis->list);
	}
  
	return 0;
}

/* 
 * On Integrity (HP IA-64) systems, slots are found in /sys/bus/pci/slots
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
 * If there only one cabinet, chassis, bay then these fields
 * should be omitted.  We set them to zero instead as you need per machine
 * info to determine the presence of or absence of them.
 */
int integrity_parse_slot(const char *slot_name,
			 int *cabinet, 
			 int *chassis,
			 int *bay,
			 int *slot)
{
	int len = strlen(slot_name);

	*cabinet = 0;
	*bay = 0;
	*chassis = 0;
	*slot = 0;

	if (len == 5) {
		*cabinet = slot_name[0] - '0';
		slot_name++;
	}
	if (len >= 4) {
		*bay = slot_name[0] - '0';
		slot_name++;
	}
	if (len >= 3) {
		*chassis = slot_name[0] - '0';
		slot_name++;
	}
	if (len >= 2) {
		if (slot_name[0] != '0') {
			sscanf(slot_name, "%d", slot);
			return 0;
		}
		slot_name++;
	}			
	if (len >= 1) {
		*slot = slot_name[0] - '0';
	}

	return 0;
}


char *integrity_cabinet_to_str(int cabinet, char *str)
{
	if (cabinet == INTEGRITY_NONE)
		return "n/a";

	sprintf(str, "%d", cabinet);
	return str;
}

char *integrity_bay_to_str(int bay, char *str)
{
	if (bay == INTEGRITY_NONE)
		return "n/a";

	sprintf(str, "%d", bay);
	return str;
}

char *integrity_chassis_to_str(int chassis, char *str)
{
	if (chassis == INTEGRITY_NONE)
		return "n/a";

	sprintf(str, "%d", chassis);
	return str;
}

char *integrity_slot_to_str(int slot, char *str)
{
	if (slot == INTEGRITY_NONE)
		return "n/a";

	sprintf(str, "%d", slot);
	return str;
}

char *integrity_slot_str(const char *slot_name, char *str) 
{
	int cabinet, bay, chassis, slot_num;

	integrity_parse_slot(slot_name, &cabinet, &bay, &chassis, &slot_num);
	sprintf(str, "%d", slot_num);
	return str;
}
