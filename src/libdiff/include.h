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

// TODO: CONSIDER MOVING THIS TO THE GLOBAL COMMON FILE
#ifndef max
#define max(a, b) ( ((a) > (b)) ? (a) : (b) )
#endif

#define END_OF_SCRIPT 0
#define INSERTION 1
#define DELETION 2



struct edit {
	struct record *rcrd;
	struct edit *next;
	unsigned char edit;
	size_t x;
	size_t k;
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
	struct edit *ses_mem, *ses_head, *ses_tail;
};



#endif /* INCLUDE_include_h__ */
