#ifndef INCLUDE_diffhelpers_h__
#define INCLUDE_diffhelpers_h__

#include "../common.h"
#include "difftypes.h"

/*
 * These methods are all lifted pretty mercilessly from xdiff/xutils.c.
 * We'll port them ourselves later.
 */
// TODO: PORT THESE PROPERLY.
long xdl_mmfile_size(diff_mem_data *mmf);
void *xdl_mmfile_next(diff_mem_data *mmf, long *size);
void *xdl_mmfile_first(diff_mem_data *mmf, long *size);
long guess_lines(diff_mem_data *mf);

#endif

