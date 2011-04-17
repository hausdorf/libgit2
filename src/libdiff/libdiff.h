#ifndef INCLUDE_libdiff_h__
#define INCLUDE_libdiff_h__

#include "../common.h"

/*
 * Represents file data (binary or text) in memory. Often
 * instances of these structs are the direct objects that
 * diffing is performed on.
 *
 * Typedef'd as both git_diff_m_data and git_diff_m_buffer,
 * which are identical in concept and execution, but
 * conceptually different.
 */
struct git_diff_mem {
	long size;
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
	// TODO: find out if this should be a size_t
	unsigned long flags;
	git_diff_callback *callback;
};

int diff(git_diff_m_data *data1, git_diff_m_data *data2,
		git_diffresults_conf const *results_conf);

#endif
