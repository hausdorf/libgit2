#ifndef INCLUDE_libdiff_h__
#define INCLUDE_libdiff_h__

#include "../common.h"
#include "difftypes.h"

int diff(diff_mem_data *data1, diff_mem_data *data2,
		git_diffresults_conf const *results_conf);

#endif
