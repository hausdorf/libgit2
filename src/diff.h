#ifndef INCLUDE_diff_h__
#define INCLUDE_diff_h__

/* --SAMPLE prone to change based on research etc--
 *
 * diffdata_t
 * mnfile_t
 *
 * Internal representation of a diff */
typedef struct {
    char *string;
    int size;
} git_diff;

/** --SAMPLE prone to change based on requirements--
 * Takes the diff of two files and stores in diff_t struct.
 *
 * @param d The struct to store the diff info into
 * @param file1 The first file to diff
 * @param file2 The second file to diff
 *
 * @return 0 on success, error otherwise
 */
int git_diff_no_index (git_diff *diff, char *file1, char *file2);


/** --SAMPLE prone to change based on requirements--
 * Does a standard git diff
 *
 * @param diff The struct to store the diff results into
 * @param repo The repo we are performing the diff on
 *
 * @return 0 on success, error otherwise
 */
int git_diff(git_diff *diff, git_repository *repo);

#endif
