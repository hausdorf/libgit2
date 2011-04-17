#include "diff.h"

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

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff_no_index(git_diffresults_conf **results_conf, const char *filename1,
		const char *filename2)
{
	return 0;
}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff(git_diffresults_conf **results_conf, git_commit *commit,
		git_repository *repo)
{
	return 0;
}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff_cached(git_diffresults_conf **results_conf, git_commit *commit,
		git_index *index)
{
	return 0;
}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff_commits(git_diffresults_conf **results_conf, git_commit *commit1,
		git_commit *commit2)
{
	return 0;
}

