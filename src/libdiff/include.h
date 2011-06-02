/*
 * include.h - INTERNAL STUFF: structs, macros, functions, etc that
 * libdiff depends on internally, but which are usually not meant
 * for exportation.
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



struct diff_env {
	struct diff_mem *diffme1, *diffme2;
	struct record *rcrds1, *rcrds2;
	size_t num_rcrds1, num_rcrds2;
	size_t rcrds_guess1, rcrds_guess2;
	struct edit *ses_mem, *ses_head, *ses_tail;
};



#endif /* INCLUDE_include_h__ */
