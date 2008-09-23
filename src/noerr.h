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

#ifndef NOERR_H
#define NOERR_H
#include <stdio.h>

/**
 * close_noerr - close without stomping errno.
 * @fd: the file descriptor to close.
 *
 * errno is saved and restored across the call to close: if an error occurs,
 * the resulting (non-zero) errno is returned.
 */
int close_noerr(int fd);

/**
 * fclose_noerr - close without stomping errno.
 * @fp: the FILE pointer.
 *
 * errno is saved and restored across the call to fclose: if an error occurs,
 * the resulting (non-zero) errno is returned.
 */
int fclose_noerr(FILE *fp);

/**
 * unlink_noerr - unlink a file without stomping errno.
 * @pathname: the path to unlink.
 *
 * errno is saved and restored across the call to unlink: if an error occurs,
 * the resulting (non-zero) errno is returned.
 */
int unlink_noerr(const char *pathname);

#endif /* NOERR_H */
