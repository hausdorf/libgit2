#ifndef INCLUDE_diff_h__
#define INCLUDE_diff_h__

#include "git2/diff.h"

/*
 * Represents file data (binary or text) in memory.
 * Where the data variable points to user data and size is the size of that data
 * This is used to hold the data from one diff file.
 */
struct git_diff_mem {
	long size;
	char *data;
};
typedef struct git_diff_mem git_diff_m_data;
typedef struct git_diff_mem git_diff_m_buffer;

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
