/*
 * libdif.h - Houses all functions that are intended to be directly
 * accessible by all of libgit2. Includes all data structures needed
 * to call and run diff. General rule: if it's not in here, it's not
 * supposed to be exported (probably).
 */
#ifndef INCLUDE_libdiff_h__
#define INCLUDE_libdiff_h__

#include "../common.h"



struct diff_mem {
	char *data;
	size_t size;
};



int diff(struct diff_mem *diffme1, struct diff_mem *diffme2);



#endif /* INCLUDE_libdiff_h__ */
