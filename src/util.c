/*
 * HP IA-64 LBA utilization monitor
 *
 * Copyright (C) 2007 Hewlett-Packard Co
 * Copyright (C) 2007 Andrew Patterson <andrew.patterson@hp.com>
 * Copyright (C) 2007 Rick Jones <rick.jones2@hp.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/limits.h>

#include "util.h"

static char prg_name[PATH_MAX] = "";

const char *program_name(void)
{
	return prg_name;
};

void set_program_name(const char *name)
{
	strncpy(prg_name, basename(name), PATH_MAX); 
}

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

	fprintf(stderr, "%s: ", program_name());
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

void fatal_error(char *fmt, ...) 
{
	va_list ap;

	fprintf(stderr, "%s: ", program_name());
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(1);
	/* NOT REACHED */
}

int read_attribute(const char *path, char *contents, size_t n)
{
	static char *buf = 0;

	int fd;
	int retval = 1;
	ssize_t amt_read;
	size_t page_size = (size_t)getpagesize();
	
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
	strncpy(contents, buf, n < page_size ? n : page_size);
	retval = 0;

fd:
	close(fd);

out:
	return retval;
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

