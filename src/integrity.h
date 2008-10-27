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
#ifndef __PCI_INTEGRITY_H__
#define __PCI_INTEGRITY_H__
 
#include <stdlib.h>
#include <string.h>

#include "list.h"

#define INTEGRITY_CABINET_NAME_SIZE 2
#define INTEGRITY_CHASSIS_NAME_SIZE 2
#define INTEGRITY_BAY_NAME_SIZE 2
#define INTEGRITY_SLOT_NAME_SIZE 2
#define INTEGRITY_NONE -1

struct cabinet_info {
	int number;
	struct list_node list;
};

struct bay_info {
	int number;
	struct list_node list;
};

struct chassis_info {
	int number;
	struct list_node list;
};

struct integrity_sys_info {
	int num_cabinets;
	struct list_head cabinet_list;
	int num_bays;
	struct list_head bay_list;
	int num_chassis;
	struct list_head chassis_list;
	int num_slots;
};

struct integrity_lba_info {
	int cabinet;
	int bay;
	int chassis;
	int num_slots;
};

int integrity_init_sys_info(struct integrity_sys_info *sys_info);
int integrity_update_lba_info(struct integrity_lba_info *info, int slot_num);
int integrity_update_sys_info(struct integrity_lba_info *lba_info,
										struct integrity_sys_info *sys_info);
int integrity_parse_slot(const char *slot_name,
		   int *cabinet, 
		   int *chassis,
		   int *bay,
		   int *slot);

static inline int integrity_match_cabinet(char *cabinet_name, int cabinet_num)
{
	return !(atoi(cabinet_name) == cabinet_num);
};
static inline int integrity_match_bay(char *bay_name, int bay_num) 
{
	return !(atoi(bay_name) == bay_num);
};
static inline int integrity_match_chassis(char *chassis_name, int chassis_num) 
{
	return !(atoi(chassis_name) == chassis_num);
};
static inline int integrity_match_slot(char *slot_name, int slot_num) 
{
	return !(atoi(slot_name) == slot_num);
};

char *integrity_cabinet_to_str(int cabinet, char *str);
char *integrity_bay_to_str(int bay, char *str);
char *integrity_chassis_to_str(int bay, char *str);
char *integrity_slot_to_str(int slot, char *str);
char *integrity_slot_str(const char *slot_name, char *str);

#endif /* __PCI_INTEGRITY_H__ */
