/*
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2,
 * as published by the Free Software Foundation.
 *
 * In addition to the permissions in the GNU General Public License,
 * the authors give you unlimited permission to link the compiled
 * version of this file into combinations with other programs,
 * and to distribute those combinations without any restriction
 * coming from the use of this file.  (The General Public License
 * restrictions do apply in other respects; for example, they cover
 * modification of the file, and distribution when not linked into
 * a combined executable.)
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "libdiff.h"
#include "diffhelpers.h"
#include "memstore.h"



static void free_classifier(record_classifier *cf);
static int init_record_classifier(record_classifier *classifier, long size);
static int prepare_data_ctx(diff_mem_data *data1, data_context *data_ctx,
		diff_environment *diff_env);
static int classify_record(record_classifier *classifier, diff_record **rhash,
		unsigned int hbits, diff_record *rec);



// TODO: THIS IS A DIRECT PORT FROM xdiff/xprepare.c
// PORT IT PROPERLY
static void free_classifier(record_classifier *cf) {

	free(cf->classd_hash);
	memstore_free(&cf->table_mem);
}


// TODO: THIS IS A DIRECT PORT FROM xdiff/xprepare.c
// PORT IT PROPERLY
static int classify_record(record_classifier *classifier, diff_record **rhash,
		unsigned int hbits, diff_record *record)
{
	long hash_idx;
	char const *line;
	classd_record *rcrec;

	line = record->data;
	// Turn record's raw hash value into an index for the hash table
	hash_idx = (long) XDL_HASHLONG(record->hash, classifier->hbits);
	// Look through the elements at that hash location; break if we actually find
	// parameter "record" at that location
	for(rcrec = classifier->classd_hash[hash_idx]; rcrec; rcrec = rcrec->next)
		if(rcrec->hash == record->hash && record_match(rcrec->line, rcrec->size,
				record->data, record->size, classifier->flags))
			break;

	// If we've NOT found "record" in classifier's hash table, insert it:
	if(!rcrec) {
		if(!(rcrec = memstore_alloc(&classifier->table_mem))) {
			return -1;
		}
		// Increment count of elements in classifier's hash table
		rcrec->idx = classifier->count++;
		rcrec->line = line;
		rcrec->size = record->size;
		rcrec->hash = record->hash;
		// next and rcrec will point to the same location
		rcrec->next = classifier->classd_hash[hash_idx];
		classifier->classd_hash[hash_idx] = rcrec;
	}

	// Insert record into the hash table
	record->hash = (unsigned long) rcrec->idx;

	hash_idx = (long) XDL_HASHLONG(record->hash, hbits);
	record->next = rhash[hash_idx];
	rhash[hash_idx] = record;

	return 0;
}


// TODO: COMMENT HERE
// TODO: COMPACT THIS METHOD -- WHAT CAN BE LEFT OUT?
static int prepare_data_ctx(diff_mem_data *data, data_context *data_ctx,
		diff_environment *diff_env)
{
	record_classifier *classifier = &diff_env->classifier;
	long guessed_len = data_ctx->guessed_size;

	long i;
	unsigned int hbits;
	long num_recs, table_size, tmp_tbl_size;
	unsigned long hash_val;
	char const *blk, *cur, *top, *prev;
	diff_record *curr_record;
	diff_record **records, **reallocd_records;
	diff_record **records_hash;
	unsigned long *hshd_recs;
	char *weights;
	long *keys;

	// Allocate memory for the hash table of records
	if(memstore_init(&data_ctx->table_mem, sizeof(diff_record),
			(guessed_len >> 2) + 1) < 0) {

		return -1;
	}
	if(!(records = (diff_record **) malloc(guessed_len * sizeof(diff_record *)))) {

		memstore_free(&data_ctx->table_mem);
		return -1;
	}

	// Build a hashtable of diff_record pointers
	//
	// 1. Start by finding hashtable size -- the smallest power of 2 greater
	// than guessed_len, the guessed number of records
	hbits = hashbits((unsigned int) guessed_len);
	table_size = 1 << hbits;
	// 2. Allocate memory required to store this table
	// TODO: MAKE SURE THIS WAS CORRECT, BECAUSE THERE WAS ORIGINALLY A MISTAKE
	// HERE. I originally wrote:
	//if(!(records_hash = (diff_record **) malloc(guessed_len *
	if(!(records_hash = (diff_record **) malloc(table_size *
			sizeof(diff_record *)))) {

		free(records);
		memstore_free(&data_ctx->table_mem);
		return -1;
	}
	// 3. Set every pointer in table to NULL
	// TODO: FIND OUT IF THIS SHOULD BE DONE WITH MEMSET INSTEAD
	// I don't think you can with a double pointer
	for(i = 0; i < table_size; i++) {
		records_hash[i] = NULL;
	}

	// TODO: guessed_len is the *guessed* number of records (read: lines)
	// why calculate the lines again here?
	num_recs = 0;
	if((cur = blk = diff_mem_first(data, &tmp_tbl_size)) != NULL) {
		for(top = blk + tmp_tbl_size;;) {
			if(cur >= top) {
				if(!(cur = blk = diff_mem_next(data, &tmp_tbl_size))) {
					break;
				}
				top = blk + tmp_tbl_size;
			}
			prev = cur;
			// FIXME:
			// THIS
			// IS
			// IM
			// POR
			// TANT.
			// RIGHT NOW WE HAVE NO HELPER METHODS TO INITIALIZE THE git_diffresults_conf
			// STRUCTS. THIS CAUSES THIS FLAG VARIABLE TO NOT BE ASSIGNED. THIS CAUSES
			// A SEGFAULT. THIS NEEDS TO BE IMPLEMENTED.
			hash_val = hash_record(&cur, top, 0 /**diff_env->flags*/);

			// if number of records is greater than guessed length of records,
			// realloc the amount of memory needed
			if(num_recs >= guessed_len) {
				guessed_len *= 2;
				if(!(reallocd_records = (diff_record **) realloc(records,
						guessed_len * sizeof(diff_record *)))) {

					free(records_hash);
					free(records);
					memstore_free(&data_ctx->table_mem);
					return -1;
				}
				records = reallocd_records;
			}

			// append a record to the hashtable
			if(!(curr_record = memstore_alloc(&data_ctx->table_mem))) {

				free(records_hash);
				free(records);
				memstore_free(&data_ctx->table_mem);
				return -1;
			}
			curr_record->data = prev;
			curr_record->size = (long) (cur-prev);
			curr_record->hash = hash_val;
			records[num_recs++] = curr_record;

			// FIXME: CLASSIFY RECORD AND THE LIKE DO NOT APPEAR TO DO ANYTHING
			if (classify_record(classifier, records_hash, hbits,
					curr_record) < 0) {

				free(records_hash);
				free(records);
				memstore_free(&data_ctx->table_mem);
				return -1;
			}
		}
	}

	// alloc space for weights array
	if (!(weights = (char *) malloc((num_recs + 2) * sizeof(char)))) {

		free(records_hash);
		free(records);
		memstore_free(&data_ctx->table_mem);
		return -1;
	}
	memset(weights, 0, (num_recs + 2) * sizeof(char));

	// alloc space for keys array
	if (!(keys = (long *) malloc((num_recs + 1) * sizeof(long)))) {

		free(weights);
		free(records_hash);
		free(records);
		memstore_free(&data_ctx->table_mem);
		return -1;
	}
	if (!(hshd_recs = (unsigned long *) malloc((num_recs + 1) * sizeof(unsigned long)))) {

		free(keys);
		free(weights);
		free(records_hash);
		free(records);
		memstore_free(&data_ctx->table_mem);
		return -1;
	}

	data_ctx->num_recs = num_recs;
	data_ctx->recs = records;
	data_ctx->hbits = hbits;
	data_ctx->rhash = records_hash;
	// QUESTION: does this create a memory leak? We're not pointing to the same spot,
	// and as far as I know, nothing else points here.
	data_ctx->weights = weights + 1;
	data_ctx->keys = keys;
	data_ctx->nreff = 0;
	data_ctx->hshd_recs = hshd_recs;
	data_ctx->dstart = 0;
	data_ctx->dend = num_recs - 1;

	return 0;
}


// TODO: COMMENT HERE
// TODO: COMPACT THIS METHOD -- WHAT CAN WE CUT OUT?
static int init_record_classifier(record_classifier *classifier, long size)
{
	long i;

	/// TODO: FIND OUT WHY IT'S size/4+1
	// Allocate memory for the hash table
	if (memstore_init(&classifier->table_mem, sizeof(classd_record),
			(size >> 2) + 1) < 0) {
		return -1;
	}

	// Build hashtable of classd_record pointers
	classifier->hbits = hashbits((unsigned int) size);
	classifier->table_size = 1 << classifier->hbits;
	if(!(classifier->classd_hash = (classd_record **) malloc(
			classifier->table_size * sizeof(classd_record *)))) {
		memstore_free(&classifier->table_mem);
		return -1;
	}

	// Zero out the hash table memory
	for(i = 0; i < classifier->table_size; i++) {
		classifier->classd_hash[i] = NULL;
	}

	classifier->count = 0;

	return 0;
}


// TODO: COMMENT THIS FUNCTION
int algo_environment(diff_environment *diff_env)
{
	// The raw content we're diffing
	diff_mem_data *data1 = diff_env->data1;
	diff_mem_data *data2 = diff_env->data2;

	// Represents context we use in the diffing algorithm
	data_context *data_ctx1 = &diff_env->data_ctx1;
	data_context *data_ctx2 = &diff_env->data_ctx2;

	record_classifier *classifier = &diff_env->classifier;

	// TODO: WE GUESS TOTAL LINES IN data1 AND data2, BUT LATER
	// ON, WE ACTUALLY FIND THE SPECIFIC TOTAL LINES IN BOTH;
	// ARE BOTH OF THESE PROCESSES NECESSARY?
	// TODO: FIND OUT WHAT THE EFF THESE MAGICAL "+1"s do.
	// Guess the size of both data1 and data2, and combine in a special way
	// to come up with the size "guess" we use to initialize things like
	// the record_classifier
	long total_size_guess = (data_ctx1->guessed_size = guess_lines(data1) + 1) +
		(data_ctx2->guessed_size = guess_lines(data2) + 1) + 1;

	if(init_record_classifier(classifier, total_size_guess) < 0) {
		return -1;
	}

	if(prepare_data_ctx(data1, data_ctx1, diff_env) < 0)
		return 0;

	if(prepare_data_ctx(data2, data_ctx2, diff_env) < 0)
		return 0;

	// FIXME: Is this smart to do? It's a member of diff_env, which means
	// it's getting carried around the interface. Do we want to haul NULL
	// around?
	free_classifier(classifier);

	// TODO TODO TODO: Patience diff will require that we optimize
	// these contexts for it. The following is the code for this
	// optimization from xdl_prepare_env
	//if (!(xpp->flags & XDF_PATIENCE_DIFF) &&
	//		xdl_optimize_ctxs(&xe->xdf1, &xe->xdf2) < 0) {

	//	xdl_free_ctx(&xe->xdf2);
	//	xdl_free_ctx(&xe->xdf1);
	//	return -1;
	//}

	return 0;
}


// TODO: COMMENT THIS FUNCTION
int prepare_and_myers(diff_environment *diff_env)
{
	// TODO: COMMENT THESE VARS
	long L;

	long *k_diags;
	long *k_diags_fwd;
	long *k_diags_bkwd;

	myers_conf conf;

	// TODO: FIND OUT HOW TO CONSOLIDATE diffdata, implement these.
	// Not needed particularly until the end of the function.
	//diffdata dd1, dd2;

	if(algo_environment(diff_env) < 0) {
		return -1;
	}

	// TODO: THIS FUNCTION IS NOT DONE YET

	return 0;
}


// TODO: COMMENT THIS FUNCTION
// TODO: IMPLEMENT THIS
int diff(diff_mem_data *data1, diff_mem_data *data2,
		git_diffresults_conf const *results_conf)
{
	// TODO COMMENT THESE VARS
	git_changeset *diff;
	diff_environment diff_env;

	// Put the data we're diffing into the diff_environment
	diff_env.data1 = data1;
	diff_env.data2 = data2;

	// Put the flags into the diff_environment
	diff_env.flags = &results_conf->flags;

	// TODO: ERROR CHECK THIS ASSIGNMENT???
	diff_results_hndlr process_results = results_conf->results_hndlr ?
		(diff_results_hndlr)results_conf->results_hndlr :
		// TODO: IMPLEMENT the default_results_hndlr; it's in
		// xdiff/xemit.c
		//default_results_hndlr;
		NULL;

	// TODO: IMPLEMENT PATIENCE DIFF
//	if(results_conf->flags & DO_PATIENCE_DIFF)
//		if(prepare_and_patience(data1, data2, &diff_env) < 0)
//			return -1;

	if(prepare_and_myers(&diff_env) < 0) {
		return -1;
	}

	// TODO: IMPLEMENT THESE
	// STEP 2: COMPACT AND BUILD SCRIPT
	// STEP 3: CALL THE SPECIFIED CALLBACK FUNCTION
	return 0;
}


/*********************
 * Patience diff lives here
 * Thar be dragons!
 *    -- and unfinished stuff, too.
 ********************/

#define NON_UNIQUE ULONG_MAX

/*
 * This is a hash mapping from line hash to line numbers in the first
 * and second file/blob
 */
struct hashmap {
    // TODO: Figure out what alloc does
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
/**
 * Insert record entries
 * @param line The line number
 * @param map The hashmap to store the entries
 * @param pass Which diff file/blob: 1 for first, 2 for second
 */
static void insert_record(int line, struct hashmap *map, int pass)
{
	/* Assign the records from the appropriate pass */
	diff_record **records = pass == 1 ? map->env->data_ctx1.recs
										: map->env->data_ctx2.recs;
	/* Grab the record corresponding to the line we are looking at */
	diff_record *record = records[line - 1], *other;

	/*
	 * After xdl_prepare_env() (or more precisely, due to
	 * xdl_classify_record()), the "ha" member of the records (AKA lines)
	 * is _not_ the hash anymore, but a linearized version of it.  In
	 * other words, the "ha" member is guaranteed to start with 0 and
	 * the second record's ha can only be 0 or 1, etc.
	 *
	 * So we multiply ha by 2 in the hope that the hashing was
	 * "unique enough".
	 */
	size_t index = (size_t)((record->hash << 1) % map->alloc);

	while (map->entries[index].line1) {
		/*
		 * Set other to the record corresponding to the line we are on
		 * This seems to be comparing file1 to file1 at times
		 * If we are on pass = 1, then diff_record will be equal to
		 * data_ctx1->recs[line-1], which other gets set to here
		 * TODO: see if this can be bypassed once
		 */
		other = map->env->data_ctx1.recs[map->entries[index].line1 - 1];
		if (map->entries[index].hash != record->hash ||
				!record_match(record->data, record->size,
					other->data, other->size,
					map->results_conf->flags)) {
			if (++index >= map->alloc)
				index = 0;
			continue;
		}
		if (pass == 2)
			map->has_matches = 1;
		if (pass == 1 || map->entries[index].line2)
			map->entries[index].line2 = NON_UNIQUE;
		else
			map->entries[index].line2 = line;

		/* If all the entries match, bail early */
		return;
	}

	/* No need to store entries if we are on the second pass */
	if (pass == 2)
		return;

	/* Map line and hash */
	map->entries[index].line1 = line;
	map->entries[index].hash = record->hash;

	/* Set first entry, if not set */
	if (!map->first)
		map->first = map->entries + index;

	/* Update linked list, setting last and next to last */
	if (map->last) {
		map->last->next = map->entries + index;
		map->entries[index].previous = map->last;
	}

	map->last = map->entries + index;
	map->record_count++;
}

/*
 * PORTED xdiff/xpatience.c from git
 * This function has to be called for each recursion into the inter-hunk
 * parts, as previously non-unique lines can become unique when being
 * restricted to a smaller part of the files.
 *
 * It is assumed that env has been prepared using xdl_prepare().
 *
 * @param data1 First blob of data, from a file or elsewhere (may not be needed)
 * @param data2 First blob of data, from a file or elsewhere (may not be needed)
 * @param results_conf We'll use the flags from this
 * @param env Already contains data[12]
 * @param result Hashmap to fill
 * @param line1 The starting line from data1
 * @param count1 The number of lines in data1
 * @param line2 The starting line from data2
 * @param count2 The number of lines in data2
 *
 * @return 0 if successful, -1 if fail
 */
static int fill_hashmap(diff_mem_data *data1, diff_mem_data *data2,
		git_diffresults_conf const *results_conf,
		diff_environment *env, struct hashmap *result,
		int line1, int count1, int line2, int count2)
{
	/*
	 * If env already has data1/2, then there is no reason to pass
	 * in two data structs
	 */
	result->file1 = data1; /* maybe? result->file1 = env->data1 */
	result->file2 = data2; /* "" */
	result->results_conf = results_conf;
	result->env = env;

	/* We know exactly how large we want the hash map */
	result->alloc = count1 * 2;
	result->entries = (struct entry *)
		ld_malloc(result->alloc * sizeof(struct entry));

	if (!result->entries)
		return -1;
	
	memset(result->entries, 0, result->alloc * sizeof(struct entry));

	/* First, fill with entries from the first file */
	while (count1--)
		insert_record(line1++, result, 1);

	/* Then search for matches in the second file */
	while (count2--)
		insert_record(line2++, result, 2);

	return 0;
}

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 * Find the longest sequence with a smaller last element (meaning a smaller
 * line2, as we construct the sequence with entries ordered by line1).
 */
static int binary_search(struct entry **sequence, int longest,
		struct entry *entry)
{
	int left = -1, right = longest;

	while (left + 1 < right) {
		int middle = (left + right) / 2;
		/* by construction, no two entries can be equal */
		if (sequence[middle]->line2 > entry->line2)
			right = middle;
		else
			left = middle;
	}
	/* return the index in "sequence", not the sequence length */
	return left;
}

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 * The idea is to start with the list of common unique lines sorted by
 * the order in file1.  For each of these pairs, the longest (partial)
 * sequence whose last element's line2 is smaller is determined.
 *
 * For efficiency, the sequences are kept in a list containing exactly one
 * item per sequence length: the sequence with the smallest last
 * element (in terms of line2).
 */
static struct entry *find_longest_common_sequence(struct hashmap *map)
{
	struct entry **sequence =
		ld_malloc(map->record_count * sizeof(struct entry *));
	size_t longest = 0, i;
	struct entry *entry;

	/*
	 * Could we use fewer comparisons by making this a while loop?
	 * entry = map->first
	 * while (entry->next) {...
	 * ?
	 */
	for (entry = map->first; entry; entry = entry->next) {
		if (!entry->line2 || entry->line2 == NON_UNIQUE)
			continue;
		i = binary_search(sequence, longest, entry);
		entry->previous = i < 0 ? NULL : sequence[i];
		sequence[++i] = entry;
		if (i == longest)
			longest++;
	}

	/* No common unique lines were found */
	if (!longest) {
		ld_free(sequence);
		return NULL;
	}

	/* Adjust the 'next' members by iterating backwards */
	entry = sequence[longest - 1];
	entry->next = NULL;
	while (entry->previous) {
		entry->previous->next = entry;
		entry = entry->previous;
	}
	ld_free(sequence);

	return entry;
}

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 * calls xdl_recmatch
 */
static int match(struct hashmap *map, int line1, int line2)
{
    return 0;
}

/*
	static int patience_diff(mmfile_t *file1, mmfile_t *file2,
	xpparam_t const *xpp, xdfenv_t *env,
	int line1, int count1, int line2, int count2);
*/
/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 * Recursively find the longest common sequence of unique lines,
 * and if none was found, ask xdl_do_diff() to do the job.
 *
 * This function assumes that env was prepared with xdl_prepare_env().
 */
static int patience_diff(diff_mem_data *file1, diff_mem_data *file2,
		git_diffresults_conf const *results_conf,
		diff_environment *env,
		int line1, int count1, int line2, int count2)
{
    return 0;
}

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 */
static int walk_common_sequence(struct hashmap *map, struct entry *first,
		int line1, int count1, int line2, int count2)
{
    return 0;
}

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 */
static int fall_back_to_classic_diff(struct hashmap *map,
		int line1, int count1, int line2, int count2)
{
    return 0;
}

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 * Entry point to patience diff
 */
/*
	int xdl_do_patience_diff(mmfile_t *file1, mmfile_t *file2,
	xpparam_t const *xpp, xdfenv_t *env)
	*/
int xdl_do_patience_diff(diff_mem_data *file1, diff_mem_data *file2,
		git_diffresults_conf const *results_conf, diff_environment *env)
{
    return 0;
}

