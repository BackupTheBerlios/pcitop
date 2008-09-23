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

#include "noerr.h"
#include <unistd.h>
#include <errno.h>

int close_noerr(int fd)
{
	int saved_errno = errno, ret;

	if (close(fd) != 0)
		ret = errno;
	else
		ret = 0;

	errno = saved_errno;
	return ret;
}

int fclose_noerr(FILE *fp)
{
	int saved_errno = errno, ret;

	if (fclose(fp) != 0)
		ret = errno;
	else
		ret = 0;

	errno = saved_errno;
	return ret;
}

int unlink_noerr(const char *pathname)
{
	int saved_errno = errno, ret;

	if (unlink(pathname) != 0)
		ret = errno;
	else
		ret = 0;

	errno = saved_errno;
	return ret;
}
