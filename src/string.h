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

#ifndef CCAN_STRING_H
#define CCAN_STRING_H
#include <string.h>
#include <stdbool.h>

/**
 * streq - Are two strings equal?
 * @a: first string
 * @b: first string
 *
 * This macro is arguably more readable than "!strcmp(a, b)".
 *
 * Example:
 *	if (streq(str, ""))
 *		printf("String is empty!\n");
 */
#define streq(a,b) (strcmp((a),(b)) == 0)

/**
 * strneq - Are two strings equal, checking no more than n characters?
 * @a: first string
 * @b: first string
 * @c: count
 *
 * This macro is arguably more readable than "!strncmp(a, b,c)".
 *
 * Example:
 *	if (strneq(str, "abc", 10))
 *		printf("String is the abc's!\n");
 */
#define strneq(a,b,c) (strncmp((a),(b),(c)) == 0)

/**
 * strstarts - Does this string start with this prefix?
 * @str: string to test
 * @prefix: prefix to look for at start of str
 *
 * Example:
 *	if (strstarts(str, "foo"))
 *		printf("String %s begins with 'foo'!\n", str);
 */
#define strstarts(str,prefix) (strncmp((str),(prefix),strlen(prefix)) == 0)

/**
 * strends - Does this string end with this postfix?
 * @str: string to test
 * @postfix: postfix to look for at end of str
 *
 * Example:
 *	if (strends(str, "foo"))
 *		printf("String %s end with 'foo'!\n", str);
 */
static inline bool strends(const char *str, const char *postfix)
{
	if (strlen(str) < strlen(postfix))
		return false;

	return streq(str + strlen(str) - strlen(postfix), postfix);
}

/**
 * strsplit - Split string into an array of substrings
 * @ctx: the context to tallocate from (often NULL)
 * @string: the string to split
 * @delims: delimiters where lines should be split.
 * @nump: optional pointer to place resulting number of lines
 *
 * This function splits a single string into multiple strings.  The
 * original string is untouched: an array is allocated (using talloc)
 * pointing to copies of each substring.  Multiple delimiters result
 * in empty substrings.  By definition, no delimiters will appear in
 * the substrings.
 *
 * The final char * in the array will be NULL, so you can use this or
 * @nump to find the array length.
 *
 * Example:
 *	unsigned int count_long_lines(const char *text)
 *	{
 *		char **lines;
 *		unsigned int i, long_lines = 0;
 *
 *		// Can only fail on out-of-memory.
 *		lines = strsplit(NULL, string, "\n", NULL);
 *		for (i = 0; lines[i] != NULL; i++)
 *			if (strlen(lines[i]) > 80)
 *				long_lines++;
 *		talloc_free(lines);
 *		return long_lines;
 *	}
 */
char **strsplit(const void *ctx, const char *string, const char *delims,
		 unsigned int *nump);

/**
 * strjoin - Join an array of substrings into one long string
 * @ctx: the context to tallocate from (often NULL)
 * @strings: the NULL-terminated array of strings to join
 * @delim: the delimiter to insert between the strings
 *
 * This function joins an array of strings into a single string.  The
 * return value is allocated using talloc.  Each string in @strings is
 * followed by a copy of @delim.
 *
 * Example:
 *	// Append the string "--EOL" to each line.
 *	char *append_to_all_lines(const char *string)
 *	{
 *		char **lines, *ret;
 *		unsigned int i, num, newnum;
 *
 *		lines = strsplit(NULL, string, "\n", NULL);
 *		ret = strjoin(NULL, lines, "-- EOL\n");
 *		talloc_free(lines);
 *		return ret;
 *	}
 */
char *strjoin(const void *ctx, char *strings[], const char *delim);

/**
 * grab_fd - read all of a file descriptor into memory
 * @ctx: the context to tallocate from (often NULL)
 * @fd: the file descriptor to read from
 * @size: the (optional) size of the file
 *
 * This function reads from the given file descriptor until no more
 * input is available.  The content is talloced off @ctx, and the size
 * of the file places in @size if it's non-NULL.  For convenience, the
 * byte after the end of the content will always be NUL.
 *
 * Example:
 *	// Return all of standard input, as lines.
 *	char **read_as_lines(void)
 *	{
 *		char **lines, *all;
 *
 *		all = grab_fd(NULL, 0, NULL);
 *		if (!all)
 *			return NULL;
 *		lines = strsplit(NULL, all, "\n", NULL);
 *		talloc_free(all);
 *		return lines;
 *	}
 */
void *grab_fd(const void *ctx, int fd, size_t *size);

/**
 * grab_file - read all of a file (or stdin) into memory
 * @ctx: the context to tallocate from (often NULL)
 * @filename: the file to read (NULL for stdin)
 * @size: the (optional) size of the file
 *
 * This function reads from the given file until no more input is
 * available.  The content is talloced off @ctx, and the size of the
 * file places in @size if it's non-NULL.  For convenience, the byte
 * after the end of the content will always be NUL.
 *
 * Example:
 *	// Return all of a given file, as lines.
 *	char **read_as_lines(const char *filename)
 *	{
 *		char **lines, *all;
 *
 *		all = grab_file(NULL, filename, NULL);
 *		if (!all)
 *			return NULL;
 *		lines = strsplit(NULL, all, "\n", NULL);
 *		talloc_free(all);
 *		return lines;
 *	}
 */
void *grab_file(const void *ctx, const char *filename, size_t *size);
#endif /* CCAN_STRING_H */
