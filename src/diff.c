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
#include "diff.h"

typedef struct {
	// begin..end of sequences a, b
	const long *begin_a, *end_a, *begin_b, *end_b;
	// the difference value of begin b, a
	const long *k;
} middle_edit;

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff_no_index(git_diffresults_conf **results_conf, const char *filename1,
		const char *filename2) {}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff(git_diffresults_conf **results_conf, git_commit *commit,
		git_repository *repo) {}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff_cached(git_diffresults_conf **results_conf, git_commit *commit,
		git_index *index) {}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff_commits(git_diffresults_conf **results_conf, git_commit *commit1,
		git_commit *commit2) {}

int xdl_recs_cmp(git_diffresults_conf *dd1, long off1, long lim1,
		 git_diffresults_conf *dd2, long off2, long lim2,
		 long *kvdf, long *kvdb, int need_min/*, xdalgoenv_t *xenv*/) {
	unsigned long const *ha1 = dd1->hashed_records, *ha2 = dd2->hashed_records;

	/*
	 * Shrink the box by walking through each diagonal snake (SW and NE).
	 */
	for (; off1 < lim1 && off2 < lim2 && ha1[off1] == ha2[off2]; off1++, off2++);
	for (; off1 < lim1 && off2 < lim2 && ha1[lim1 - 1] == ha2[lim2 - 1]; lim1--, lim2--);

	/*
	 * If one dimension is empty, then all records on the other one must
	 * be obviously changed.
	 */
	/*if (off1 == lim1) {
		char *rchg2 = dd2->rchg;
		long *rindex2 = dd2->rindex;

		for (; off2 < lim2; off2++)
			rchg2[rindex2[off2]] = 1;
	} else if (off2 == lim2) {
		char *rchg1 = dd1->rchg;
		long *rindex1 = dd1->rindex;

		for (; off1 < lim1; off1++)
			rchg1[rindex1[off1]] = 1;
	} else {
		xdpsplit_t spl;
		spl.i1 = spl.i2 = 0;*/

		/*
		 * Divide ...
		 */
		/*if (xdl_split(ha1, off1, lim1, ha2, off2, lim2, kvdf, kvdb,
			      need_min, &spl, xenv) < 0) {

			return -1;
		}*/

		/*
		 * ... et Impera.
		 */
		/*if (xdl_recs_cmp(dd1, off1, spl.i1, dd2, off2, spl.i2,
				 kvdf, kvdb, spl.min_lo, xenv) < 0 ||
		    xdl_recs_cmp(dd1, spl.i1, lim1, dd2, spl.i2, lim2,
				 kvdf, kvdb, spl.min_hi, xenv) < 0) {

			return -1;
		}
	}*/

	return 0;
}

/*int main()
{
	git_diff_data dd1, dd2;
	long off1, lim1, off2, lim2, kvdf, kvdb;
	int need_min;
	xdl_recs_cmp(&dd1, off1, lim1, &dd2, off2, lim2, &kvdf, &kvdb, need_min/ *, xdalgoenv_t *xenv* /);
}*/

