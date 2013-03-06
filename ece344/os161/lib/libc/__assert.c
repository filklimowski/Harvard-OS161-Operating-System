#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * Function that gets called when an assert() fails.
 * Print a message to stderr and bail out of the program.
 */

void
__bad_assert(const char *file, int line, const char *expr)
{
	int i, len;
	char buf[256];
	snprintf(buf, sizeof(buf), "Assertion failed: %s (%s line %d)\n",
		 expr, file, line);

	for (i=0; i<strlen(buf); i++)
		len=printchar(buf[i]);

	abort();
}
