#ifndef INCLUDE_libdiff_h__
#define INCLUDE_libdiff_h__

#include "../common.h"
#include "difftypes.h"



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
