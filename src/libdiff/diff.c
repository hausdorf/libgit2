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
#include <math.h>
#include "include.h"
#include "libdiff.h"
#include "environment.h"



#define GUESS_NEWLINES 256



struct record * make_rcrds(struct diff_mem *mem)
{
	int i, tmp, num_rcrds;
	const int size = mem->size;
	const char *data = mem->data;
	int guess = mem->num_recs_guess;

	// allocate space for the record list

	for (;;) {
		tmp = i;
		// find next newline
		for (; i <= size, data[++i] != '\n';)
			;
		// create and hash this record
		struct record
		//hash_rcrd();
		if (++num_rcrds >= guess)
			; // realloc

	}
}


long guess_num_rcrds(struct diff_mem *content)
{
	// TODO TODO TODO: This function is pretty much from libxdiff, and just
	// counts the new lines, more or less. Can we do better? I think so.

	long nwlines = 0, size, tmpsz = 0;
	char const *data, *curr, *end;

	if ((curr = data = content->data) != NULL) {
		size = content->size;

		// Increment newlines for every '\n' we find; break
		// when curr passes end
		for (end = data + size; nwlines < GUESS_NEWLINES;) {
			if (curr >= end) {
				tmpsz += (long) (curr - data);
				curr = data = NULL;
				break;
			}
			nwlines++;

			if (!(curr = memchr(curr, '\n', end - curr)))
				curr = end;
			else
				curr++;
		}
		tmpsz += (long) (curr - data);
	}

	if(nwlines && tmpsz)
		nwlines = content->size / (tmpsz / nwlines);

	return nwlines + 1;
}


int prepare_data(struct diff_env *env)
{
	struct diff_mem *diffme1 = env->diffme1, *diffme2 = env->diffme2;

	// Guess # of records in data; we malloc the space needed to build list of
	// records and record hashes
	diffme1->num_recs_guess = guess_num_rcrds(env->diffme1);
	diffme2->num_recs_guess = guess_num_rcrds(env->diffme1);

	make_rcrds(env->diffme1);

	return 0;
}


void build_script(int *v, int v_size, char *s1, char *s2, int d, int m, int n, int k)
{
	// TODO TODO TODO: See if we can consolidate the normalization to be with the
	// regular loop??


	// set x and y to be the last respective characters of s1 and s2
	int x = v[k];
	int y = x - k;
	x -= 2; y -= 2;

	// myers() terminates greedily when we find the first path from (0,0) to (N,M),
	// so k, x, and y, may have been in the middle of myers() inner loop when this
	// func was called. We need to normalize them:
	while (x >= 0 && y >= 0 && s1[x] == s2[y]) {

		printf("DIAGONAL %c %c\n", s1[x], s2[y]);
		x--;
		y--;
	}

	// Return if documents are the same
	// TODO: we probably should not even call this function, in this case
	if (x < 0 && y < 0) {

		printf("RETURNING\n");
		return;
	}

	// Continue "normalization"
	if (k == -d || (k != d && v[k-1] < v[k+1])) {

		printf("INSERTION\t%c %c\t%c %c\n", s1[x], s2[y], s1[x], s2[y-1]);
		k++;
		y--;
		d--;
	}
	else {

		printf("DELETION\t%c %c\t%c %c\n", s1[x], s2[y], s1[x-1], s2[y]);
		k--;
		x--;
		d--;
	}

	// Return if there is one difference
	if (x < 0 && y < 0) { printf("RETURNING FIRST\n"); return; }

	// Move to previous version of V
	v -= v_size;

	// Do the above once for every version of V in v_hstry
	for (; d >= 0; d--, v -= v_size) {

		while (x >= 0 && y >= 0 && s1[x] == s2[y] ) {

			printf("dIAGONAL %c %c %c %c\n", s1[x], s2[y], s1[x-1], s2[y-1]);
			x--;
			y--;
		}

		if (x < 0 && y < 0) {

			printf("RETURNING\n");
			return;
		}

		if (k == -d || (k != d && v[k-1] < v[k+1])) {

			printf("iNSERTION\t%c %c\t%c %c\n", s1[x], s2[y], s1[x], s2[y-1]);
			k++;
			y--;
		}
		else {

			printf("dELETION\t%c %c\t%c %c\n", s1[x], s2[y], s1[x-1], s2[y]);
			k--;
			x--;
		}
	}
	printf("ERROR\n");
}


int myers(struct diff_env *env)
{
	// For the most part, we use the *exact same* variable names that
	// Myers uses in his original paper.
	/*
	char *s1 = "abcabba";
	char *s2 = "cbabac";
	int m = 7, n = 6;
	*/

	/*
	char *s1 = "cowby";
	char *s2 = "cwboy";
	int m = 5, n = 5;
	*/

	/*
	char *s1 = "cow";
	char *s2 = "cow";
	int m = 3, n = 3;
	*/

	char *s1 = "cow";
	char *s2 = "ow";
	int m = 3, n = 2;

	int max = m + n;

	// Set up Myers' "V" array; v goes in the middle of v_mem so that
	// we can use negative indices exactly as the paper does
	int v_size = (max*2+1);              // total elements in V
	int v_bytes = sizeof(int) * v_size;  // size in bytes of V

	int *v_mem = malloc(v_bytes);
	int *v = v_mem+max;

	// Alloc memory to save a copy of each version of Myers' "V" array
	int *v_hstry_mem = malloc(pow(v_bytes, 2));
	int *v_hstry = v_hstry_mem;

	int x, y;
	v[1] = 0;  // IMPORTANT -- V's seed value

	int d, k;
	for (d = 0; d <= max; d++)
	{
		for (k = -d; k <= d; k += 2)
		{
			// Determine whether we're moving right or down in edit graph
			if (k == -d || (k != d && v[k - 1] < v[k + 1])) {
				x = v[k + 1];
			}
			else {
				x = v[k - 1] + 1;
			}
			y = x - k;

			// Make sure we're not stepping outside of the string bounds
			if (x > m) { x = m; }
			if (y > n) { y = n; }

			// Skip over the diagonals, if any
			while (x <= m && y <= n && s1[x] == s2[y]) {
				x++; y++;
			}

			// Record current endpoint for current k line
			v[k] = x;

			memcpy(v_hstry, v_mem, v_bytes);

			v_hstry += v_size;

			// Greedily terminate if we've found a path that works
			if (x >= m && y >= n) {
				printf("RESULT: %d\n", d);
				build_script(v_hstry - v_size + max, v_size, s1, s2, d, m, n, k);
				return 0;
			}

		}
	}

	return 0;
}


int prepare_and_myers(struct diff_env *env)
{
	prepare_data(env);
	myers(env);

	return 0;
}


int diff(struct diff_mem *diffme1, struct diff_mem *diffme2)
{
	struct diff_env env;

	// Init environment, put files to diff inside
	if (init_diff_env(&env, diffme1, diffme2) < 0)
		return -1;

	if (prepare_and_myers(&env) < 0)
		return -1;

	return 0;
}

