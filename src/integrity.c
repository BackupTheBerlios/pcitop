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
#include "integrity.h"

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

	*cabinet = INTEGRITY_NONE;
	*bay = INTEGRITY_NONE;
	*chassis = INTEGRITY_NONE;
	*slot = INTEGRITY_NONE;

	if (len == 5) {
		*cabinet = slot_name[0] + '0';
		slot_name++;
	}
	if (len >= 4) {
		*bay = slot_name[0] + '0';
		slot_name++;
	}
	if (len >= 3) {
		*chassis = slot_name[0] + '0';
		slot_name++;
	}
	if (len >= 2) {
		if (slot_name[0] != '0') {
			sscanf("%02u", slot_name, slot);
			return 0;
		}
		slot_name++;
	}			
	if (len >= 1) {
		*slot = slot_name[0] + '0';
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
