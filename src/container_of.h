/* 
   Comprehensive C Archive Network -- http://ccan.ozlabs.org

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef CCAN_CONTAINER_OF_H
#define CCAN_CONTAINER_OF_H
#include <stddef.h>

#include "config.h"
#include "check_type.h"

/**
 * container_of - get pointer to enclosing structure
 * @member_ptr: pointer to the structure member
 * @containing_type: the type this member is within
 * @member: the name of this member within the structure.
 *
 * Given a pointer to a member of a structure, this macro does pointer
 * subtraction to return the pointer to the enclosing type.
 *
 * Example:
 *	struct info
 *	{
 *		int some_other_field;
 *		struct foo my_foo;
 *	};
 *
 *	struct info *foo_to_info(struct foo *foop)
 *	{
 *		return container_of(foo, struct info, my_foo);
 *	}
 */
#define container_of(member_ptr, containing_type, member)		\
	 ((containing_type *)						\
	  ((char *)(member_ptr) - offsetof(containing_type, member))	\
	  - check_types_match(*(member_ptr), ((containing_type *)0)->member))


/**
 * container_of_var - get pointer to enclosing structure using a variable
 * @member_ptr: pointer to the structure member
 * @var: a pointer to a structure of same type as this member is within
 * @member: the name of this member within the structure.
 *
 * Given a pointer to a member of a structure, this macro does pointer
 * subtraction to return the pointer to the enclosing type.
 *
 * Example:
 *	struct info
 *	{
 *		int some_other_field;
 *		struct foo my_foo;
 *	};
 *
 *	struct info *foo_to_info(struct foo *foop)
 *	{
 *		struct info *i = container_of_var(foo, i, my_foo);
 *		return i;
 *	}
 */
#ifdef HAVE_TYPEOF
#define container_of_var(member_ptr, var, member) \
	container_of(member_ptr, typeof(*var), member)
#else
#define container_of_var(member_ptr, var, member) \
	((void *)((char *)(member_ptr) - offsetof(containing_type, member)))
#endif

#endif /* CCAN_CONTAINER_OF_H */
