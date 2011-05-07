/*
 * include.h - Collection of miscellaneous items (usually
 * definitions) that we use a lot here in libdiff:
 * #includes for common header files, declarations for
 * common structs, and so on.
 */
#ifndef INCLUDE_include_h__
#define INCLUDE_include_h__

#include "../common.h"
#include "libdiff.h"



#define ld__malloc(x) malloc(x)
#define ld__realloc(ptr, x) realloc(ptr, x)
#define ld__free(x) free(x)



struct record {
	unsigned long start;
	unsigned long end;
	unsigned long hash;
};


struct diff_env {
	struct diff_mem *diffme1, *diffme2;
	struct record *rcrds1, *rcrds2;
};



#endif /* INCLUDE_include_h__ */
