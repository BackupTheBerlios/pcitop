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

#define INTEGRITY_CABINET_NAME_SIZE 2
#define INTEGRITY_CHASSIS_NAME_SIZE 2
#define INTEGRITY_BAY_NAME_SIZE 2
#define INTEGRITY_SLOT_NAME_SIZE 2
#define INTEGRITY_NONE -1

struct integrity_sys_info {
	int num_cabinets;
	int num_bays;
	int num_chassis;
};

struct integrity_lba_info {
	int cabinet;
	int bay;
	int chassis;
};

int integrity_parse_slot(const char *slot_name,
		   int *cabinet, 
		   int *chassis,
		   int *bay,
		   int *slot);

static inline int integrity_match_cabinet(char *cabinet_name, int cabinet_num)
{
	return (atoi(cabinet_name) == cabinet_num);
};
static inline int integrity_match_bay(char *bay_name, int bay_num) 
{
	return (atoi(bay_name) == bay_num);
};
static inline int integrity_match_chassis(char *chassis_name, int chassis_num) 
{
	return (atoi(chassis_name) == chassis_num);
};
static inline int integrity_match_slot(char *slot_name, int slot_num) 
{
	return (atoi(slot_name) == slot_num);
};

char *integrity_cabinet_to_str(int cabinet, char *str);
char *integrity_bay_to_str(int bay, char *str);
char *integrity_chassis_to_str(int bay, char *str);
char *integrity_slot_to_str(int slot, char *str);

#endif /* __PCI_INTEGRITY_H__ */
