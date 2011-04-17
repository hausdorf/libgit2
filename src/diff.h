#ifndef INCLUDE_diff_h__
#define INCLUDE_diff_h__

#include "git2/diff.h"

struct git_diffresults_conf {
	unsigned long flags;
};

// TODO: REMOVE
// Keeping this because it's a good readable reference of what
// the corresponding xdiff/git struct is SUPPOSED to do.
/*struct git_diffresults_conf {
	// The number of records (lines) in the content
	long num_records;
	// Ordered list of hashed records
	unsigned long const *hashed_records;
	// The index of the record in the content
	long *record_index;
	// The set of records that have changed, represented by 1: changed
	char *records_changed;
};*/

#endif
