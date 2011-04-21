#ifndef INCLUDE_libdiff_h__
#define INCLUDE_libdiff_h__

#include "../common.h"
#include "difftypes.h"



// TODO: COMMENT HERE
// TODO: IS THIS STRUCT EVEN NECESSARY?
struct diffdata {
	long num_recs;
	// This is DEFINITELY an array of hashes; in
	// xdl_recs_cmp we access it as such
	unsigned long const *hshd_recs;
	// Points to diff_context.keys: Array of longs that represent hashes,
	// used to access weights[]
	long *keys;
	// Points to diff_context.weights: In xdl_recs_cmp, each of these
	// chars is set to a "weight" -- usually 0 or 1 depending on whether
	// that particular record has changed. So it's basically a bit vector
	// made of chars
	char *weights;
};
typedef struct diffdata diffdata;

/*
 * Represents the inner variables required to run Myers O(ND)
 * diffing algorithm. This includes guiding information like
 * the maximum cost we're willing to incur before we switch
 * to a slightly different version of O(ND).
 */
struct myers_conf {
	// We use this to determine when our LCS traversal has
	// become too expensive, at which point we switch to
	// something else
	long maxcost;
	long snake_cnt;
	long heur_min;
};
typedef struct myers_conf myers_conf;



int diff(diff_mem_data *data1, diff_mem_data *data2,
		git_diffresults_conf const *results_conf);

#endif
