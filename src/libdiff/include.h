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



#define ld__malloc(x) git__malloc(x)
#define ld__realloc(ptr, x) git__realloc(ptr, x)
#define ld__free(x) free(x)

#define INSERTION 0
#define DELETION 1



struct edit {
	struct record *rcrd;
	struct edit *prev, *next;
	unsigned char edit;
	size_t x;
};


struct record {
	unsigned long start;
	unsigned long end;
	unsigned long hash;
};


struct diff_env {
	struct diff_mem *diffme1, *diffme2;
	struct record *rcrds1, *rcrds2;
	size_t num_rcrds1, num_rcrds2;
	size_t rcrds_guess1, rcrds_guess2;
	struct edit *edt_scrpt;
};



#endif /* INCLUDE_include_h__ */
