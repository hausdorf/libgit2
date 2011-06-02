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



// Everything needed to perform diff
struct diff_env {

	// Content to diff; we will produce the minimum number of
	// changes req'd to change diffme1 into diffme2
	struct diff_mem *diffme1, *diffme2;

	// Arrays of records; obtained by processing diffme1 and
	// diffme2, respectively
	struct record *rcrds1, *rcrds2;

	// Number of records for diffme1 and diffme2, respectively
	size_t num_rcrds1, num_rcrds2;

	// Quickly-computed guess of number of records in diffme1 & 2
	size_t rcrds_guess1, rcrds_guess2;

	// SES = Shortest Edit Script; array of changes req'd to
	// change diffme1 into diffme2.
	struct edit *ses_mem;

	// Head and tail of SES
	struct edit *ses_head, *ses_tail;
};



#endif /* INCLUDE_include_h__ */
