#include <stdio.h>

/*
 * C standard I/O function - print a string and a newline.
 */

int
puts(const char *s)
{
	__puts(s);
	printchar('\n');
	return 0;
}
