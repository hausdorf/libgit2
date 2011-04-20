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



static int init_record_classifier(record_classifier *classifier, long size);
static int prepare_data_ctx(diff_mem_data *data1, long guessed_len,
		data_context *data_ctx, diff_environment *diff_env);



// TODO: COMMENT HERE
// TODO: COMPACT THIS METHOD -- WHAT CAN BE LEFT OUT?
static int prepare_data_ctx(diff_mem_data *data1, long guessed_len,
		data_context *data_ctx, diff_environment *diff_env)
{
	unsigned int hbits;
	long i, num_recs, table_size;
	diff_record **records;
	diff_record **records_hash;

	// Allocate memory for the hash table of records
	if(memstore_init(&data_ctx->table_mem, sizeof(diff_record),
			guessed_len / 4 + 1) < 0)
	{
		return -1;
	}
	// TODO: FIGURE OUT WTF THIS BIT DOES -- I FORGET
	if(!(records = (diff_record **) malloc(guessed_len *
			sizeof(diff_record *))))
	{
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
	if(!(records_hash = (diff_record **) malloc(guessed_len *
			sizeof(diff_record *))))
	{
		free(records);
		memstore_free(&data_ctx->table_mem);
		return -1;
	}
	// 3. Set every pointer in table to NULL
	// TODO: FIND OUT IF THIS SHOULD BE DONE WITH MEMSET INSTEAD
	for(i = 0; i < table_size; i++)
	{
		records_hash[i] = NULL;
	}

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
			size / 4 + 1) < 0)
	{
		return -1;
	}

	// Build hashtable of classd_record pointers
	classifier->hbits = hashbits((unsigned int) size);
	classifier->table_size = 1 << classifier->hbits;
	if(!(classifier->classd_hash = (classd_record **) malloc(
			classifier->table_size * sizeof(classd_record *))))
	{
		memstore_free(&classifier->table_mem);
		return -1;
	}

	// Zero out the hash table memory
	for(i = 0; i < classifier->table_size; i++)
	{
		classifier->classd_hash[i] = NULL;
	}

	classifier->count = 0;

	return 0;
}

// TODO: COMMENT THIS FUNCTION
int myers_environment(diff_mem_data *data1, diff_mem_data *data2,
		diff_environment *diff_env)
{
	// TODO: REMOVE ME WHEN YOU'RE DONE DEVELOPING THE ALGORITHM
	printf("MYERS_ENVIRONMENT\n");

	long guess1, guess2;
	record_classifier classifier;

	// TODO: WE GUESS TOTAL LINES IN data1 AND data2, BUT LATER
	// ON, WE ACTUALLY FIND THE SPECIFIC TOTAL LINES IN BOTH;
	// ARE BOTH OF THESE PROCESSES NECESSARY?
	// TODO: FIND OUT WHAT THE EFF THESE MAGICAL "+1"s do.
	guess1 = guess_lines(data1) + 1;
	guess2 = guess_lines(data2) + 1;

	if(init_record_classifier(&classifier, guess1 + guess2 + 1) < 0)
	{
		return -1;
	}

	/*if(prepare_ctx(data1, guess_lines1, diff_env->flags, &diff_env->data_ctx1) < 0)
		return 0;*/


	// TODO: THIE METHOD IS NOT DONE YET. IMPLEMENT IT.

	return 0;
}

// TODO: COMMENT THIS FUNCTION
int prepare_and_myers(diff_mem_data *data1, diff_mem_data *data2,
		diff_environment *diff_env)
{
	// TODO: REMOVE ME WHEN YOU'RE DONE DEVELOPING THE ALGORITHM
	printf("PREPARE_AND_MYERS\n");

	// TODO: COMMENT THESE VARS
	long L;

	long *k_diags;
	long *k_diags_fwd;
	long *k_diags_bkwd;

	myers_conf conf;

	// TODO: FIND OUT HOW TO CONSOLIDATE diffdata, implement these.
	// Not needed particularly until the end of the function.
	//diffdata dd1, dd2;

	if(myers_environment(data1, data2, diff_env) < 0)
	{
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
	// TODO: REMOVE ME WHEN YOU'RE DONE DEVELOPING THE ALGORITHM
	printf("DIFF\n");

	// TODO COMMENT THESE VARS
	git_changeset *diff;
	diff_environment diff_env;
	// TODO: ERROR CHECK THIS ASSIGNMENT???
	diff_results_hndlr process_results = results_conf->results_hndlr ?
		(diff_results_hndlr)results_conf->results_hndlr :
		// TODO: IMPLEMENT the default_results_hndlr; it's in
		// xdiff/xemit.c
		//default_results_hndlr;
		NULL;

	diff_env.flags = &results_conf->flags;

	// TODO: IMPLEMENT PATIENCE DIFF
//	if(results_conf->flags & DO_PATIENCE_DIFF)
//		if(prepare_and_patience(data1, data2, &diff_env) < 0)
//			return -1;

	if(prepare_and_myers(data1, data2, &diff_env) < 0)
	{
		return -1;
	}

	// TODO: IMPLEMENT THESE
	// STEP 2: COMPACT AND BUILD SCRIPT
	// STEP 3: CALL THE SPECIFIED CALLBACK FUNCTION
	return 0;
}

