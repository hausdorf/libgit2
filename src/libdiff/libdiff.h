#ifndef INCLUDE_libdiff_h__
#define INCLUDE_libdiff_h__

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

int diff(git_diff_m_data *data1, git_diff_m_data *data2,
		git_diffresults_conf const *results_conf);

#endif
