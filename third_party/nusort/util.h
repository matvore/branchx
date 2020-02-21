/* Taken from github.com/matvore/nusort - used with permission */

#ifndef NUSORT_UTIL_H
#define NUSORT_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xreallocarray(void *ptr, size_t count, size_t el_size);

/*
 * GROW_ARRAY_BY
 *
 * Create a struct that looks like this:
 *
 * struct array {
 * 	TYPE *el;
 * 	size_t cnt;
 * 	size_t alloc;
 * };
 *
 * Where TYPE is any type you want. The number of elements will be kept in 'cnt'
 * and the capacity in 'alloc'.
 */
#define GROW_ARRAY_BY(array, grow_cnt) do { \
	(array).cnt += grow_cnt; \
	if ((array).alloc < (array).cnt) { \
		size_t old_alloc = (array).alloc; \
		(array).alloc *= 2; \
		if ((array).alloc < (array).cnt) \
			(array).alloc = (array).cnt; \
		(array).el = xreallocarray((array).el, (array).alloc, \
			sizeof(*(array).el)); \
		memset((array).el + old_alloc, 0, \
			((array).alloc - old_alloc) * sizeof(*(array).el)); \
	} \
} while (0)

#define DESTROY_ARRAY(array) do { \
	FREE((array).el); \
	(array).cnt = 0; \
	(array).alloc = 0; \
} while (0)

#define FREE(ptr) do { \
	free(ptr); \
	(ptr) = NULL; \
} while (0)

#define DIE(show_errno, ...) \
do { \
	if (show_errno) { \
		perror("Fatal error"); \
		fputc('\t', stderr); \
	} else { \
		fputs("Fatal error ", stderr); \
	} \
	fprintf(stderr, "at %s:%ld\n", __FILE__, (long) __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	fputc('\n', stderr); \
	exit(224); \
} while(0)

#endif
