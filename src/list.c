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

#include <stdio.h>
#include <stdlib.h>
#include "list.h"

struct list_head *list_check(struct list_head *h, const char *abortstr)
{
	struct list_node *n, *p;
	int count = 0;

	if (h->n.next == &h->n) {
		if (h->n.prev != &h->n) {
			if (!abortstr)
				return NULL;
			fprintf(stderr, "%s: prev corrupt in empty %p\n",
				abortstr, h);
			abort();
		}
		return h;
	}

	for (p = &h->n, n = h->n.next; n != &h->n; p = n, n = n->next) {
		count++;
		if (n->prev != p) {
			if (!abortstr)
				return NULL;
			fprintf(stderr,
				"%s: prev corrupt in node %p (%u) of %p\n",
				abortstr, n, count, h);
			abort();
		}
	}
	return h;
}
