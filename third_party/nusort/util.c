/* Taken from github.com/matvore/nusort - used with permission */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "third_party/nusort/util.h"

void *xreallocarray(void *ptr, size_t count, size_t el_size)
{
	ptr = realloc(ptr, count * el_size);
	if (!ptr && count)
		DIE(0, "out of memory");
	return ptr;
}
