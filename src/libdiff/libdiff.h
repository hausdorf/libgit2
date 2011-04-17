#ifndef INCLUDE_libdiff_h__
#define INCLUDE_libdiff_h__

// TODO: find out if Doxygen comments should be used internally like so
/*!
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

#endif
