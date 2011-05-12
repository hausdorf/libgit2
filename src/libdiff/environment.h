/*
 * environment.h - Holds miscellaneous builders, destructors, and
 * and allocators that make life easier for users of libdiff.
 */
#ifndef INCLUDE_environment_h__
#define INCLUDE_environment_h__

#include "include.h"



int init_diff_env(struct diff_env *env, struct diff_mem *diffme1,
		struct diff_mem *diffme2);
void free_edt_scrpt(struct edit *script);
void free_rcrds(struct record *r);
void free_env(struct diff_env *env);



#endif /* INCLUDE_environment_h__ */
