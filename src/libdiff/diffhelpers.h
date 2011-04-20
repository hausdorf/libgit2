#ifndef INCLUDE_diffhelpers_h__
#define INCLUDE_diffhelpers_h__

#include "../common.h"
#include "difftypes.h"

/*
 * These methods are all lifted pretty mercilessly from xdiff/xutils.c.
 * We'll port them ourselves later.
 */
// TODO: PORT THESE PROPERLY.
static unsigned long hash_record_with_whitespace(char const **data,
		char const *top, long flags);
unsigned long hash_record(char const **data, char const *top, long flags);
unsigned int hashbits(unsigned int size);
long diff_mem_size(diff_mem_data *mmf);
void *diff_mem_next(diff_mem_data *mmf, long *size);
void *diff_mem_first(diff_mem_data *mmf, long *size);
long guess_lines(diff_mem_data *mf);

#endif

