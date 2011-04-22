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



// Default values for Myers algorithm parameters
#define XDL_MAX_COST_MIN 256
#define XDL_HEUR_MIN_COST 256
#define XDL_SNAKE_CNT 20
#define XDL_K_HEUR 4



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
 * Basically considers a "box" (off1, off2, lim1, lim2) and scan from both
 * the forward diagonal starting from (off1, off2) and the backward diagonal
 * starting from (lim1, lim2). If the K values on the same diagonal crosses
 * returns the furthest point of reach. We might end up having to expensive
 * cases using this algorithm is full, so a little bit of heuristic is needed
 * to cut the search and to return a suboptimal point.
 */
static long myers(unsigned long const *ha1, long off1, long lim1,
		      unsigned long const *ha2, long off2, long lim2,
		      long *kvdf, long *kvdb, int need_min, split *spl,
		      myers_conf *xenv) {
	long dmin = off1 - lim2, dmax = lim1 - off2;
	long fmid = off1 - off2, bmid = lim1 - lim2;
	long odd = (fmid - bmid) & 1;
	long fmin = fmid, fmax = fmid;
	long bmin = bmid, bmax = bmid;
	// IMPORTANT: ec is the EDIT COST. THIS IS WHAT WE ARE RETURNING.
	// d is the D-path
	long ec, d, i1, i2, prev1, best, dd, v, k;

	/*
	// Set initial diagonal values for both forward and backward path.
	kvdf[fmid] = off1;
	kvdb[bmid] = lim1;

// BEGIN REQUIRED COMPONENTS TO IMPLEMENT ALGORITHM

	for (ec = 1;; ec++) {
		int got_snake = 0;

		// We need to extent the diagonal "domain" by one. If the next
		// values exits the box boundaries we need to change it in the
		// opposite direction because (max - min) must be a power of two.
		// Also we initialize the external K value to -1 so that we can
		// avoid extra conditions check inside the core loop.
		if (fmin > dmin)
			kvdf[--fmin - 1] = -1;
		else
			++fmin;
		if (fmax < dmax)
			kvdf[++fmax + 1] = -1;
		else
			--fmax;

		// Greedy LCS/SES algorithm: see Myers, page 6, it's almost 1-1
		// mapping for the Myers algorithm
		//
		// INITIALIZE D-PATH: we start at M+N and work backwards, rather
		// than starting at the beginning. In this way we avoid having
		// to lookahead.
		for (d = fmax; d >= fmin; d -= 2) {
			if (kvdf[d - 1] >= kvdf[d + 1])
				i1 = kvdf[d - 1] + 1;
			else
				i1 = kvdf[d + 1];
			prev1 = i1;
			i2 = i1 - d;
			for (; i1 < lim1 && i2 < lim2 && ha1[i1] == ha2[i2]; i1++, i2++);
			if (i1 - prev1 > xenv->snake_cnt)
				got_snake = 1;
			kvdf[d] = i1;
			if (odd && bmin <= d && d <= bmax && kvdb[d] <= i1) {
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
			kvdb[--bmin - 1] = XDL_LINE_MAX;
		else
			++bmin;
		if (bmax < dmax)
			kvdb[++bmax + 1] = XDL_LINE_MAX;
		else
			--bmax;

		for (d = bmax; d >= bmin; d -= 2) {
			if (kvdb[d - 1] < kvdb[d + 1])
				i1 = kvdb[d - 1];
			else
				i1 = kvdb[d + 1] - 1;
			prev1 = i1;
			i2 = i1 - d;
			for (; i1 > off1 && i2 > off2 && ha1[i1 - 1] == ha2[i2 - 1]; i1--, i2--);
			if (prev1 - i1 > xenv->snake_cnt)
				got_snake = 1;
			kvdb[d] = i1;
			if (!odd && fmin <= d && d <= fmax && i1 <= kvdf[d]) {
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
		if (got_snake && ec > xenv->heur_min) {
			for (best = 0, d = fmax; d >= fmin; d -= 2) {
				dd = d > fmid ? d - fmid: fmid - d;
				i1 = kvdf[d];
				i2 = i1 - d;
				v = (i1 - off1) + (i2 - off2) - dd;

				if (v > XDL_K_HEUR * ec && v > best &&
				    off1 + xenv->snake_cnt <= i1 && i1 < lim1 &&
				    off2 + xenv->snake_cnt <= i2 && i2 < lim2) {
					for (k = 1; ha1[i1 - k] == ha2[i2 - k]; k++)
						if (k == xenv->snake_cnt) {
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
				i1 = kvdb[d];
				i2 = i1 - d;
				v = (lim1 - i1) + (lim2 - i2) - dd;

				if (v > XDL_K_HEUR * ec && v > best &&
				    off1 < i1 && i1 <= lim1 - xenv->snake_cnt &&
				    off2 < i2 && i2 <= lim2 - xenv->snake_cnt) {
					for (k = 0; ha1[i1 + k] == ha2[i2 + k]; k++)
						if (k == xenv->snake_cnt - 1) {
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
		if (ec >= xenv->mxcost) {
			long fbest, fbest1, bbest, bbest1;

			fbest = fbest1 = -1;
			for (d = fmax; d >= fmin; d -= 2) {
				i1 = XDL_MIN(kvdf[d], lim1);
				i2 = i1 - d;
				if (lim2 < i2)
					i1 = lim2 + d, i2 = lim2;
				if (fbest < i1 + i2) {
					fbest = i1 + i2;
					fbest1 = i1;
				}
			}

			bbest = bbest1 = XDL_LINE_MAX;
			for (d = bmax; d >= bmin; d -= 2) {
				i1 = XDL_MAX(off1, kvdb[d]);
				i2 = i1 - d;
				if (i2 < off2)
					i1 = off2 + d, i2 = off2;
				if (i1 + i2 < bbest) {
					bbest = i1 + i2;
					bbest1 = i1;
				}
			}

			if ((lim1 + lim2) - bbest < fbest - (off1 + off2)) {
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
	*/
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
/*
 * This is a hash mapping from line hash to line numbers in the first
 * and second file/blob
 */
struct hashmap {
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
	} *entires, *first, *last;

	/* were common records found? */
	size_t has_matches;
	diff_mem_data *file1, *file2;
	diff_environment *env;
	git_diffresults_conf const results_conf;
};
/**
 * Insert record entries
 * @param line The line number
 * @param map The hashmap to store the entries
 * @param which Which diff file/blob: 1 for first, 2 for second
 */
static void insert_record(int line, struct hashmap *map, int which);

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
		int line1, int count1, int line2, int count2);

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 * Find the longest sequence with a smaller last element (meaning a smaller
 * line2, as we construct the sequence with entries ordered by line1).
 */
static int binary_search(struct entry **sequence, int longest,
		struct entry *entry);

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
static struct entry *find_longest_common_sequence(struct hashmap *map);

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 * calls xdl_recmatch
 */
static int match(struct hashmap *map, int line1, int line2);

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
		int line1, int count1, int line2, int count2);

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 */
static int walk_common_sequence(struct hashmap *map, struct entry *first,
		int line1, int count1, int line2, int count2);

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 */
static int fall_back_to_classic_diff(struct hashmap *map,
		int line1, int count1, int line2, int count2);

/*
 * PORTED DIRECTLY FROM xdiff with only modifications to the types
 * Entry point to patience diff
 */
/*
	int xdl_do_patience_diff(mmfile_t *file1, mmfile_t *file2,
	xpparam_t const *xpp, xdfenv_t *env)
	*/
int xdl_do_patience_diff(diff_mem_data *file1, diff_mem_data *file2,
		git_diffresults_conf const *results_conf, diff_environment *env);

