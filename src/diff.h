#ifndef INCLUDE_diff_h__
#define INCLUDE_diff_h__

#include "git2/diff.h"

/*
 * Represents file data (binary or text) in memory. Often
 * instances of these structs are the direct objects that
 * diffing is performed on.
 *
 * Typedef'd as both git_diff_m_data and git_diff_m_buffer,
 * which are identical in execution, but conceptually different
 * and follow a different pipeline.
 */
struct git_diff_mem {
	size_t size;
	char *data;
};
typedef struct git_diff_mem git_diff_m_data;
typedef struct git_diff_mem git_diff_m_buffer;

/*
 * Callback function; called directly after we perform the
 * diff and the output is assembled according to options
 * specified in the git_diff_out_conf struct.
 */
struct git_diff_callback {
	void *payload;
	int (*out_func)(void *, git_diff_m_buffer *, int);
};
typedef struct git_diff_callback git_diff_callback;

/*
 * Fully configures not just how we generate diff's results,
 * but also *where* they go, and what form they take.
 */
struct git_diffresults_conf {
    // unique flags correspond to specific bit positions in 'flags'
	unsigned long flags;
	git_diff_callback *callback;
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
