#ifndef INCLUDE_libdiff_h__
#define INCLUDE_libdiff_h__

#include "../common.h"
#include "difftypes.h"


// TODO: COMMENT HERE
// TODO: IS THIS STRUCT EVEN NECESSARY?
struct parsed_data {
	long num_recs;
	// This is DEFINITELY an array of hashes; in
	// xdl_recs_cmp we access it as such
	unsigned long const *hshd_recs;
	// Points to diff_context.keys: Array of longs that represent hashes,
	// used to access weights[]
	long *keys;
	// Points to diff_context.weights: In xdl_recs_cmp, each of these
	// chars is set to a "weight" -- usually 0 or 1 depending on whether
	// that particular record has changed. So it's basically a bit vector
	// made of chars
	char *weights;
};
typedef struct parsed_data parsed_data;

// TODO: COMMENT HERE
// TODO: This may be a silly struct
struct split {
	// i1 and i2 are x and y per the Myers O(ND) paper
	long i1, i2;
	// DEPRECATED: git does not use the flag XDF_NEEDS_MINIMAL anymore, so these
	// do not get used
	int min_lo, min_hi;
};
typedef struct split split;


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


// libdiff memory allocation by default is handled through libgit's
#define ld__malloc(x) git__malloc(x)
#define ld__free(ptr) free(ptr)
#define ld__realloc(ptr,x) git__realloc(ptr,x)


int diff(diff_mem_data *data1, diff_mem_data *data2,
		git_diffresults_conf const *results_conf);

/*
 * This is a hash mapping from line hash to line numbers in the first
 * and second file/blob
 */
struct hashmap {
	int record_count, alloc;
	struct entry {
		size_t hash;

		/*
		 * 0 = unused entry, 1 = first line, 2 = second, etc.
		 * line2 is NON_UNIQUE if the line is not unique
		 * in either the first or the second file.
		 */
		size_t line1, line2;

		/*
		 * "next" & "previous" are used for the longest common
		 * sequence;
		 * initially, "next" reflects only the order in file1.
		 */
		struct entry *next, *previous;
	} *entries, *first, *last;

	/* were common records found? */
	size_t has_matches;
	diff_mem_data *file1, *file2;
	diff_environment *env;
	git_diffresults_conf const *results_conf;
};

/* Declare functions */
static void insert_record(int line, struct hashmap *map, int which);
static int fill_hashmap(diff_mem_data *data1, diff_mem_data *data2,
		git_diffresults_conf const *results_conf,
		diff_environment *env, struct hashmap *result,
		int line1, int count1, int line2, int count2);
static int binary_search(struct entry **sequence, int longest,
		struct entry *entry);
static struct entry *find_longest_common_sequence(struct hashmap *map);
static int match(struct hashmap *map, int line1, int line2);
static int patience_diff(diff_mem_data *file1, diff_mem_data *file2,
		git_diffresults_conf const *results_conf,
		diff_environment *env,
		int line1, int count1, int line2, int count2);
static int walk_common_sequence(struct hashmap *map, struct entry *first,
		int line1, int count1, int line2, int count2);
static int fall_back_to_classic_diff(struct hashmap *map,
		int line1, int count1, int line2, int count2);
int do_patience_diff(diff_mem_data *file1, diff_mem_data *file2,
		git_diffresults_conf const *results_conf, diff_environment *env);

#endif
