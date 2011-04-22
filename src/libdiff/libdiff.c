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



// TODO: These are MACROS; they should probably go elsewhere
#define XDL_MIN(a, b) ((a) < (b) ? (a): (b))
#define XDL_MAX(a, b) ((a) > (b) ? (a): (b))

// Default values for Myers algorithm parameters
#define XDL_MAX_COST_MIN 256
#define XDL_HEUR_MIN_COST 256
#define XDL_SNAKE_CNT 20
#define XDL_K_HEUR 4
#define XDL_LINE_MAX (long)((1UL << (CHAR_BIT * sizeof(long) - 1)) - 1)



static void free_classifier(record_classifier *cf);
static int init_record_classifier(record_classifier *classifier, long size);
static int prepare_data_ctx(diff_mem_data *data1, data_context *data_ctx,
		diff_environment *diff_env);
static int classify_record(record_classifier *classifier, diff_record **rhash,
		unsigned int hbits, diff_record *rec);



// TODO: THIS IS A DIRECT PORT FROM xdiff/xprepare.c
// PORT IT PROPERLY
static void free_ctx(data_context *ctx) {

	free(ctx->rhash);
	free(ctx->weights);
	free(ctx->keys - 1);
	free(ctx->hshd_recs);
	free(ctx->recs);
	memstore_free(&ctx->table_mem);
}


// TODO: THIS IS A DIRECT PORT FROM xdiff/xprepare.c
// PORT IT PROPERLY
static void free_classifier(record_classifier *cf) {

	free(cf->classd_hash);
	memstore_free(&cf->table_mem);
}


// TODO: WE NEED TO BE ABSOLUTELY CERTAIN THAT THIS IS CORRECT
void free_env(diff_environment *diff_env) {

	free_ctx(&diff_env->data_ctx1);
	free_ctx(&diff_env->data_ctx2);
	free_classifier(&diff_env->classifier);
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

	// Find hashtable size -- the smallest power of 2 greater than guessed_len,
	// the guessed number of records
	hbits = hashbits((unsigned int) guessed_len);
	table_size = 1 << hbits;
	// Allocate memory required to store this table
	if(!(records_hash = (diff_record **) malloc(table_size *
			sizeof(diff_record *)))) {

		free(records);
		memstore_free(&data_ctx->table_mem);
		return -1;
	}
	// Set every pointer in table to NULL
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

	// alloc space for array that will hold the hashes of every record
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


// TODO: COMMENT HERE
// TODO: This is currently a direct port from xdiff/xdiffi.c
int xdl_change_compact(data_context *xdf, data_context *xdfo, long flags) {
	long ix, ixo, ixs, ixref, grpsiz, nrec = xdf->num_recs;
	char *rchg = xdf->weights, *rchgo = xdfo->weights;
	diff_record **recs = xdf->recs;

	/*
	 * This is the same of what GNU diff does. Move back and forward
	 * change groups for a consistent and pretty diff output. This also
	 * helps in finding joinable change groups and reduce the diff size.
	 */
	for (ix = ixo = 0;;) {
		/*
		 * Find the first changed line in the to-be-compacted file.
		 * We need to keep track of both indexes, so if we find a
		 * changed lines group on the other file, while scanning the
		 * to-be-compacted file, we need to skip it properly. Note
		 * that loops that are testing for changed lines on rchg* do
		 * not need index bounding since the array is prepared with
		 * a zero at position -1 and N.
		 */
		for (; ix < nrec && !rchg[ix]; ix++)
			while (rchgo[ixo++]);
		if (ix == nrec)
			break;

		/*
		 * Record the start of a changed-group in the to-be-compacted file
		 * and find the end of it, on both to-be-compacted and other file
		 * indexes (ix and ixo).
		 */
		ixs = ix;
		for (ix++; rchg[ix]; ix++);
		for (; rchgo[ixo]; ixo++);

		do {
			grpsiz = ix - ixs;

			/*
			 * If the line before the current change group, is equal to
			 * the last line of the current change group, shift backward
			 * the group.
			 */
			while (ixs > 0 && recs[ixs - 1]->hash == recs[ix - 1]->hash &&
			       record_match(recs[ixs - 1]->data, recs[ixs - 1]->size, recs[ix - 1]->data, recs[ix - 1]->size, flags)) {
				rchg[--ixs] = 1;
				rchg[--ix] = 0;

				/*
				 * This change might have joined two change groups,
				 * so we try to take this scenario in account by moving
				 * the start index accordingly (and so the other-file
				 * end-of-group index).
				 */
				for (; rchg[ixs - 1]; ixs--);
				while (rchgo[--ixo]);
			}

			/*
			 * Record the end-of-group position in case we are matched
			 * with a group of changes in the other file (that is, the
			 * change record before the end-of-group index in the other
			 * file is set).
			 */
			ixref = rchgo[ixo - 1] ? ix: nrec;

			/*
			 * If the first line of the current change group, is equal to
			 * the line next of the current change group, shift forward
			 * the group.
			 */
			while (ix < nrec && recs[ixs]->hash == recs[ix]->hash &&
			       record_match(recs[ixs]->data, recs[ixs]->size, recs[ix]->data, recs[ix]->size, flags)) {
				rchg[ixs++] = 0;
				rchg[ix++] = 1;

				/*
				 * This change might have joined two change groups,
				 * so we try to take this scenario in account by moving
				 * the start index accordingly (and so the other-file
				 * end-of-group index). Keep tracking the reference
				 * index in case we are shifting together with a
				 * corresponding group of changes in the other file.
				 */
				for (; rchg[ix]; ix++);
				while (rchgo[++ixo])
					ixref = ix;
			}
		} while (grpsiz != ix - ixs);

		/*
		 * Try to move back the possibly merged group of changes, to match
		 * the recorded postion in the other file.
		 */
		while (ixref < ix) {
			rchg[--ixs] = 1;
			rchg[--ix] = 0;
			while (rchgo[--ixo]);
		}
	}

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


/*
 * See "An O(ND) Difference Algorithm and its Variations", by Eugene Myers.
 * Basically considers a "box" (left1, left2, right1, right2) and scan from both
 * the forward diagonal starting from (left1, left2) and the backward diagonal
 * starting from (right1, right2). If the K values on the same diagonal crosses
 * returns the furthest point of reach. We might end up having to expensive
 * cases using this algorithm is full, so a little bit of heuristic is needed
 * to cut the search and to return a suboptimal point.
 */
static long myers(unsigned long const *hshd_recs1, long left1, long right1,
		      unsigned long const *hshd_recs2, long left2, long right2,
		      long *k_fwd, long *k_bwd, int need_min, split *spl,
		      myers_conf *conf) {
	long dmin = left1 - right2, dmax = right1 - left2;
	long fmid = left1 - left2, bmid = right1 - right2;
	long odd = (fmid - bmid) & 1;
	long fmin = fmid, fmax = fmid;
	long bmin = bmid, bmax = bmid;
	// IMPORTANT: ec is the EDIT COST. THIS IS WHAT WE ARE RETURNING.
	// d is the D-path
	long ec, d, i1, i2, prev1, best, dd, v, k;

	// Set initial diagonal values for both forward and backward path.
	k_fwd[fmid] = left1;
	k_bwd[bmid] = right1;

// BEGIN REQUIRED COMPONENTS TO IMPLEMENT ALGORITHM

	for (ec = 1;; ec++) {
		int got_snake = 0;

		// We need to extent the diagonal "domain" by one. If the next
		// values exits the box boundaries we need to change it in the
		// opposite direction because (max - min) must be a power of two.
		// Also we initialize the external K value to -1 so that we can
		// avoid extra conditions check inside the core loop.
		if (fmin > dmin)
			k_fwd[--fmin - 1] = -1;
		else
			++fmin;
		if (fmax < dmax)
			k_fwd[++fmax + 1] = -1;
		else
			--fmax;

		// Greedy LCS/SES algorithm: see Myers, page 6, it's almost 1-1
		// mapping for the Myers algorithm
		//
		// INITIALIZE D-PATH: we start at M+N and work backwards, rather
		// than starting at the beginning. In this way we avoid having
		// to lookahead.
		for (d = fmax; d >= fmin; d -= 2) {
			if (k_fwd[d - 1] >= k_fwd[d + 1])
				i1 = k_fwd[d - 1] + 1;
			else
				i1 = k_fwd[d + 1];
			prev1 = i1;
			i2 = i1 - d;
			for (; i1 < right1 && i2 < right2 && hshd_recs1[i1] == hshd_recs2[i2]; i1++, i2++);
			if (i1 - prev1 > conf->snake_cnt)
				got_snake = 1;
			k_fwd[d] = i1;
			if (odd && bmin <= d && d <= bmax && k_bwd[d] <= i1) {
				spl->i1 = i1;
				spl->i2 = i2;
				spl->min_lo = spl->min_hi = 1;
				return ec;
			}
		}

		// We need to extent the diagonal "domain" by one. If the next
		// values exits the box boundaries we need to change it in the
		// opposite direction because (max - min) must be a power of two.
		// Also we initialize the external K value to -1 so that we can
		// avoid extra conditions check inside the core loop.
		if (bmin > dmin)
			k_bwd[--bmin - 1] = XDL_LINE_MAX;
		else
			++bmin;
		if (bmax < dmax)
			k_bwd[++bmax + 1] = XDL_LINE_MAX;
		else
			--bmax;

		for (d = bmax; d >= bmin; d -= 2) {
			if (k_bwd[d - 1] < k_bwd[d + 1])
				i1 = k_bwd[d - 1];
			else
				i1 = k_bwd[d + 1] - 1;
			prev1 = i1;
			i2 = i1 - d;
			for (; i1 > left1 && i2 > left2 && hshd_recs1[i1 - 1] == hshd_recs2[i2 - 1]; i1--, i2--);
			if (prev1 - i1 > conf->snake_cnt)
				got_snake = 1;
			k_bwd[d] = i1;
			if (!odd && fmin <= d && d <= fmax && i1 <= k_fwd[d]) {
				spl->i1 = i1;
				spl->i2 = i2;
				spl->min_lo = spl->min_hi = 1;
				return ec;
			}
		}

// END REQUIRED COMPONENTS TO IMPLEMENT ALGORITHM

		if (need_min)
			continue;

		// If the edit cost is above the heuristic trigger and if
		// we got a good snake, we sample current diagonals to see
		// if some of the, have reached an "interesting" path. Our
		// measure is a function of the distance from the diagonal
		// corner (i1 + i2) penalized with the distance from the
		// mid diagonal itself. If this value is above the current
		// edit cost times a magic factor (XDL_K_HEUR) we consider
		// it interesting.
		if (got_snake && ec > conf->heur_min) {
			for (best = 0, d = fmax; d >= fmin; d -= 2) {
				dd = d > fmid ? d - fmid: fmid - d;
				i1 = k_fwd[d];
				i2 = i1 - d;
				v = (i1 - left1) + (i2 - left2) - dd;

				if (v > XDL_K_HEUR * ec && v > best &&
				    left1 + conf->snake_cnt <= i1 && i1 < right1 &&
				    left2 + conf->snake_cnt <= i2 && i2 < right2) {
					for (k = 1; hshd_recs1[i1 - k] == hshd_recs2[i2 - k]; k++)
						if (k == conf->snake_cnt) {
							best = v;
							spl->i1 = i1;
							spl->i2 = i2;
							break;
						}
				}
			}
			if (best > 0) {
				spl->min_lo = 1;
				spl->min_hi = 0;
				return ec;
			}

			for (best = 0, d = bmax; d >= bmin; d -= 2) {
				dd = d > bmid ? d - bmid: bmid - d;
				i1 = k_bwd[d];
				i2 = i1 - d;
				v = (right1 - i1) + (right2 - i2) - dd;

				if (v > XDL_K_HEUR * ec && v > best &&
				    left1 < i1 && i1 <= right1 - conf->snake_cnt &&
				    left2 < i2 && i2 <= right2 - conf->snake_cnt) {
					for (k = 0; hshd_recs1[i1 + k] == hshd_recs2[i2 + k]; k++)
						if (k == conf->snake_cnt - 1) {
							best = v;
							spl->i1 = i1;
							spl->i2 = i2;
							break;
						}
				}
			}
			if (best > 0) {
				spl->min_lo = 0;
				spl->min_hi = 1;
				return ec;
			}
		}

		// Enough is enough. We spent too much time here and now we collect
		// the furthest reaching path using the (i1 + i2) measure.
		if (ec >= conf->maxcost) {
			long fbest, fbest1, bbest, bbest1;

			fbest = fbest1 = -1;
			for (d = fmax; d >= fmin; d -= 2) {
				i1 = XDL_MIN(k_fwd[d], right1);
				i2 = i1 - d;
				if (right2 < i2)
					i1 = right2 + d, i2 = right2;
				if (fbest < i1 + i2) {
					fbest = i1 + i2;
					fbest1 = i1;
				}
			}

			bbest = bbest1 = XDL_LINE_MAX;
			for (d = bmax; d >= bmin; d -= 2) {
				i1 = XDL_MAX(left1, k_bwd[d]);
				i2 = i1 - d;
				if (i2 < left2)
					i1 = left2 + d, i2 = left2;
				if (i1 + i2 < bbest) {
					bbest = i1 + i2;
					bbest1 = i1;
				}
			}

			if ((right1 + right2) - bbest < fbest - (left1 + left2)) {
				spl->i1 = fbest1;
				spl->i2 = fbest - fbest1;
				spl->min_lo = 1;
				spl->min_hi = 0;
			} else {
				spl->i1 = bbest1;
				spl->i2 = bbest - bbest1;
				spl->min_lo = 0;
				spl->min_hi = 1;
			}
			return ec;
		}
	}
}


// TODO: PUT COMMENT HERE
int recursive_compare(parsed_data *data1, long left1, long right1,
		parsed_data *data2, long left2, long right2, long *k_fwd,
		long *k_bwd, int need_min, myers_conf *conf)
{
	unsigned long const *hshd_recs1 = data1->hshd_recs,
			*hshd_recs2 = data2->hshd_recs;

	// Compare data1 and data2 from the BEGINNING; break on inequality
	for(; left1 < right1 && left2 < right2 &&
			hshd_recs1[left1] == hshd_recs2[left2]; left1++, left2++);
	// Compare data1 and data2 from the END; break on inequality
	for(; left1 < right1 && left2 < right2 &&
			hshd_recs1[right1-1] == hshd_recs2[right2-1]; right1--, right2--);

	if(left1 == right1) {
		char *weights2 = data2->weights;
		long *keys2 = data2->keys;

		// these edges are non-diagonal, so set their weight to 1
		for (; left2 < right2; left2++)
			weights2[keys2[left2]] = 1;
	} else if(left2 == right2) {
		char *weights1 = data1->weights;
		long *keys1 = data1->keys;

		// these edges are non-diagonal, so set their weight to 1
		for(; left1 < right1; left1++)
			weights1[keys1[left1]] = 1;
	} else {
		split spl;
		spl.i1 = spl.i2 = 0;

		if(myers(hshd_recs1, left1, right1, hshd_recs2, left2, right2, k_fwd, k_bwd,
				need_min, &spl, conf) < 0) {

			return -1;
		}

		if(recursive_compare(data1, left1, spl.i1, data2, left2, spl.i2,
				k_fwd, k_bwd, spl.min_lo, conf) < 0 ||
		   recursive_compare(data1, spl.i1, right1, data2, spl.i2, right2,
				k_fwd, k_bwd, spl.min_hi, conf) < 0) {

			return -1;
		}
	}

	return 0;
}


// TODO: COMMENT THIS FUNCTION
int prepare_and_myers(diff_environment *diff_env)
{
	long ndiags;    // Equivalent to Myers' "L" parameter; total
	                // combined len of both things we're diffing

	long *k_diags;  // mem for both fwd and bwd K-diagonals allocd here
	long *k_fwd;    // points to the fwd K-diag inside k_diags
	long *k_bwd;    // points to the bwd K-diag inside k_diags

	parsed_data data1, data2;

	myers_conf conf;

	// Setup and acquire information we need to perform diff
	if(algo_environment(diff_env) < 0) {
		return -1;
	}

	// Allocate memory for the forward and backward K diagonals
	ndiags = diff_env->data_ctx1.nreff + diff_env->data_ctx2.nreff + 3;
	if(!(k_diags = (long *) malloc((2 * ndiags + 2) * sizeof(long)))) {
		free_env(diff_env);
	}

	// point k_fwd and k_bwd to the appropriate location in memory allocd
	// at *k_diags
	k_fwd = k_diags;
	k_bwd = k_diags + ndiags;
	k_fwd += diff_env->data_ctx2.nreff + 1;
	k_bwd += diff_env->data_ctx2.nreff + 1;

	// set up parameters for Myers
	conf.maxcost = bogosqrt(ndiags);
	if(conf.maxcost < XDL_MAX_COST_MIN)
		conf.maxcost = XDL_MAX_COST_MIN;
	conf.snake_cnt = XDL_SNAKE_CNT;
	conf.heur_min = XDL_HEUR_MIN_COST;

	// Put data we got from algo_environment into parsed_data instances
	data1.num_recs = diff_env->data_ctx1.nreff;
	data1.hshd_recs = diff_env->data_ctx1.hshd_recs;
	data1.weights = diff_env->data_ctx1.weights;
	data1.keys = diff_env->data_ctx1.keys;
	data2.num_recs = diff_env->data_ctx2.nreff;
	data2.hshd_recs = diff_env->data_ctx2.hshd_recs;
	data2.weights = diff_env->data_ctx2.weights;
	data2.keys = diff_env->data_ctx2.keys;

	if(recursive_compare(&data1, 0, data1.num_recs, &data2, 0,
			data2.num_recs, k_fwd, k_bwd, 0, &conf) < 0) {
		free(k_diags);
		free_env(diff_env);
		return -1;
	}

	free(k_diags);

	return 0;
}


// TODO: COMMENT THIS FUNCTION
// TODO: IMPLEMENT THIS
int diff(diff_mem_data *data1, diff_mem_data *data2,
		git_diffresults_conf const *results_conf)
{
	diff_environment diff_env;
	diff_env.data1 = data1;
	diff_env.data2 = data2;
	diff_env.flags = results_conf->flags;

	git_changeset *diff;

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

	// Prepare algorithm environment and then run Myers
	if(prepare_and_myers(&diff_env) < 0) {
		return -1;
	}

	// Build edit script
	/*
	if (xdl_change_compact(&xe.xdf1, &xe.xdf2, xpp->flags) < 0 ||
	    xdl_change_compact(&xe.xdf2, &xe.xdf1, xpp->flags) < 0 ||
	    xdl_build_script(&xe, &xscr) < 0) {

		xdl_free_env(&xe);
		return -1;
	}
	if (xscr) {
		if (ef(&xe, xscr, ecb, xecfg) < 0) {

			xdl_free_script(xscr);
			xdl_free_env(&xe);
			return -1;
		}
		xdl_free_script(xscr);
	}
	xdl_free_env(&xe);
	*/

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
	// TODO: Figure out what nr and alloc do
	int nr, alloc;
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
	// Assign the records from the appropriate pass
	diff_record **records = pass == 1 ? map->env->data_ctx1.recs
										: map->env->data_ctx2.recs;
	// Grab the record corresponding to the line we are looking at
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

}

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 * This function has to be called for each recursion into the inter-hunk
 * parts, as previously non-unique lines can become unique when being
 * restricted to a smaller part of the files.
 *
 * It is assumed that env has been prepared using xdl_prepare().
 */
/*
	static int fill_hashmap(mmfile_t *file1, mmfile_t *file2,
	xpparam_t const *xpp, xdfenv_t *env,
	struct hashmap *result,
	int line1, int count1, int line2, int count2)
*/
static int fill_hashmap(diff_mem_data *data1, diff_mem_data *data2,
		git_diffresults_conf const *results_conf,
		diff_environment *env, struct hashmap *result,
		int line1, int count1, int line2, int count2)
{
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
	return 0;
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
	return NULL;
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

