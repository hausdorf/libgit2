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

// TODO: COMMENT THIS FUNCTION
int prepare_and_diff(diff_mem_data *data1, diff_mem_data *data2,
		diff_environment *diff_env)
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
	// TODO: ERROR CHECK THIS ASSIGNMENT???
	diff_results_hndlr process_results = results_conf->results_hndlr ?
		(diff_results_hndlr)results_conf->results_hndlr :
		// TODO: IMPLEMENT the default_results_hndlr; it's in
		// xdiff/xemit.c
		//default_results_hndlr;
		NULL;

	// TODO: IMPLEMENT PATIENCE DIFF
//	if(diff_env->flags & DO_PATIENCE_DIFF)
//		if(prepare_and_patience(data1, data2, &diff_env) < 0)
//			return -1;

	if(prepare_and_diff(data1, data2, &diff_env) < 0)
	{
		return -1;
	}

	// TODO: IMPLEMENT THESE
	// STEP 2: COMPACT AND BUILD SCRIPT
	// STEP 3: CALL THE SPECIFIED CALLBACK FUNCTION
	return 0;
}

