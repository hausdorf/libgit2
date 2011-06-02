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



unsigned long hash_rcrd(struct record *rcrd, const char *source)
{
	unsigned long hash = 5381;
	size_t i = rcrd->start;

	// TODO: IMPLEMENT THIS
	/*
	if (flags & XDF_WHITESPACE_FLAGS)
		return xdl_hash_record_with_whitespace(data, top, flags);
	*/

	for (; i < rcrd->end && source[i] != '\n'; i++) {
		hash += (hash << 5);
		hash ^= (unsigned long) source[i];
	}
	// Clever! TODO: find out if this is the better way?
	//*data = ptr < top ? ptr + 1: ptr;

	return hash;
}


int make_rcrds(struct record **rtrn_val, struct diff_mem *mem, size_t *num_rcrds_guess)
{
	int i, tmp;
	size_t num_rcrds = 0;
	const int data_size = mem->size;
	const char *data = mem->data;
	size_t guess = *num_rcrds_guess;

	// allocate space for the record list
	struct record *rcrds;
	if (!(rcrds = ld__malloc(sizeof(struct record) * guess))) {

		// TODO: PUT FREE() HERE
		return -1;
	}
	memset(rcrds, 0, sizeof(struct record) * guess);
	struct record *curr_rcrd = rcrds;

	// beginning to end of given data
	// TODO: TEST BUG FIX: this used to be i <= data_size; making it i < data_size
	// has eliminated a memory leak, but may or may not compromise code correctness.
	// May also need to adjust the check on the inner for loop here also
	for (i = 0; i < data_size;) {
		tmp = i;
		// find next newline
		for (; i <= data_size && data[i++] != '\n';)
			;
		// create record and record hash
		curr_rcrd->start = tmp;
		curr_rcrd->end = i;
		curr_rcrd->hash = hash_rcrd(curr_rcrd, data);

		curr_rcrd++;
		// realloc if number of records is bigger than guess
		if (++num_rcrds >= guess) {
			guess = *num_rcrds_guess = guess * 2;
			if (!(rcrds = ld__realloc(rcrds, sizeof(struct record) * guess))) {

				// TODO: ADD FREE() HERE
				return -1;
			}
		}
	}

	*rtrn_val = rcrds;

	return num_rcrds;
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
	// Guess # of records in data; we malloc the space needed to build list of
	// records and record hashes
	env->rcrds_guess1 = guess_num_rcrds(env->diffme1);
	env->rcrds_guess2 = guess_num_rcrds(env->diffme1);

	// TODO: IMPLEMENT CHECKS FOR NULL POINTERS HERE
	env->num_rcrds1 = make_rcrds(&env->rcrds1, env->diffme1, &env->rcrds_guess1);
	// TODO TODO TODO: YOU ARE HERE, IMPLEMENTING CHECKS FOR THESE
	// FUNCTION CALLS. YOU ALSO NEED TO MAKE PRINT LOOPS THAT VERIFY THAT WHAT
	// WE'RE PUTTING IN IS WHAT WE'RE GETTING OUT.
	env->num_rcrds2 = make_rcrds(&env->rcrds2, env->diffme2, &env->rcrds_guess2);

	// TODO: REMOVE. For debugging, verifies that the recorsd are copied
	/*
	size_t i;
	for (i = 0; i < env->rcrds_guess1; i++) {
		printf("%lu\t%lu\t%lu\n", env->rcrds1[i].start, env->rcrds1[i].end, env->rcrds1[i].hash);
	}

	printf("\n");

	for (i = 0; i < env->rcrds_guess2; i++) {
		printf("%lu\t%lu\t%lu\n", env->rcrds2[i].start, env->rcrds2[i].end, env->rcrds2[i].hash);
	}
	*/

	return 0;
}


// TODO DEBUGGING FUNCTION TAKE OUT
void p(struct record *r, struct diff_mem *d)
{
	char *data = d->data;
	unsigned long i;
	for (i = r->start; i < r->end; i++) {
		printf("%c", data[i]);
	}
}


// TODO DEBUGGING FUNCTION TAKE OUT
void edits(struct edit *e)
{
	for(; e->edit != END_OF_SCRIPT; e = e->next) {
		printf("EDIT: %d\n", e->edit);
	}
}


void insertion(struct edit **e, struct record *r, size_t x, size_t y, size_t k)
{
	(*e)++;
	(*e)->edit = INSERTION;
	(*e)->x = x;
	(*e)->y = y;
	(*e)->k = k;
	(*e)->rcrd = r;
	(*e)->next = *e - 1;
}


void deletion(struct edit **e, size_t x, size_t y, size_t k)
{
	(*e)++;
	(*e)->edit = DELETION;
	(*e)->x = x;
	(*e)->y = y;
	(*e)->k = k;
	(*e)->next = *e - 1;
}


void build_script(int *v, int v_size, struct diff_env *env, int d, int m, int n, int k)
{
	printf("\n\n");

	struct record *rcrds1, *rcrds2;
	struct diff_mem *diffme1, *diffme2;
	size_t x, y;
	struct edit *curr_edt;

	rcrds1 = env->rcrds1;
	rcrds2 = env->rcrds2;
	diffme1 = env->diffme1;
	diffme2 = env->diffme2;

	if (!(env->ses_mem = ld__malloc(sizeof(struct edit) * (max(m, n) + d + 1)))) {

		// TODO: WE SHOUDL RETURN AN ERROR CODE
		return;
	}

	// Seed values for edit script
	curr_edt = env->ses_mem;
	curr_edt->edit = END_OF_SCRIPT;
	env->ses_tail = curr_edt;
	env->ses_head = curr_edt;  // Updated again only just before return

	// TODO TODO TODO: See if we can consolidate the normalization to be with the
	// regular loop??

	// set x and y to be the last respective characters of s1 and s2
	x = v[k];
	y = x - k;
	x -= 1; y -= 1;

	// myers() terminates greedily when we find the first path from (0,0) to (N,M),
	// so k, x, and y, may have been in the middle of myers() inner loop when this
	// func was called. We need to normalize them:
	for (; x + 1 > x && y + 1 > y && rcrds1[x].hash == rcrds2[y].hash; x--, y--) {

		printf("DIAGONAL\n");
	}

	// Return if documents are the same
	// TODO: we probably should not even call this function, in this case
	if (x + 1 < x && y + 1 < y) {

		printf("RETURNING\n");
		return;
	}

	// Continue "normalization"
	if (k == -d || (k != d && v[k-1] < v[k+1])) {

		printf("INSERTION at x %zu y %zu\t", x, y);
		p(&rcrds2[y], diffme2);

		// Separate tmp declaration into two lines to avoid undefined
		// behavior with y-- in following line
		struct record *tmp = &rcrds2[y];
		insertion(&curr_edt, tmp, x, y--, k++);

		d--;
	}
	else {

		printf("DELETION at x %zu y %zu\t", x, y);
		p(&rcrds1[x], diffme1);

		deletion(&curr_edt, x--, y, k--);

		d--;
	}

	// Return if there is one difference
	if (x + 1 < x && y + 1 < y) {

		printf("RETURNING FIRST\n");
		env->ses_head = curr_edt;
		return;
	}

	// Move to previous version of V
	v -= v_size;

	// Do the above once for every version of V in v_hstry
	for (; d >= 0; d--, v -= v_size) {

		for (; x + 1 > x && y + 1 > y && rcrds1[x].hash == rcrds2[y].hash; x--, y--) {

			printf("dIAGONAL\n");
		}

		if (x + 1 < x && y + 1 < y) {

			printf("RETURNING\n");
			env->ses_head = curr_edt;
			return;
		}

		if (k == -d || (k != d && v[k-1] < v[k+1])) {

			printf("iNSERTION at x %zu y %zu\t", x, y);
			p(&rcrds2[y], diffme2);

			// Separate tmp declaration into two lines to avoid undefined
			// behavior with y-- in following line
			struct record *tmp = &rcrds2[y];
			insertion(&curr_edt, tmp, x, y--, k++);
		}
		else {

			printf("dELETION at x %zu y %zu\t", x, y);
			p(&rcrds1[x], diffme1);

			deletion(&curr_edt, x--, y, k--);
		}
	}
	printf("ERROR\n");
}


int myers(struct diff_env *env)
{
	int m, n, max;
	struct record *rcrds1, *rcrds2;
	int v_size, v_bytes;
	int *v_mem, *v, *v_hstry, *v_hstry_mem;
	int x, y;
	int d, k;

	rcrds1 = env->rcrds1;
	rcrds2 = env->rcrds2;
	m = env->num_rcrds1, n = env->num_rcrds2;

	max = m + n;

	printf("m %d n %d\n", m, n);

	// Set up Myers' "V" array; v goes in the middle of v_mem so that
	// we can use negative indices exactly as the paper does
	v_size = (max*2+1);              // total elements in V
	v_bytes = sizeof(int) * v_size;  // size in bytes of V

	if (!(v_mem = ld__malloc(v_bytes))) {

		// TODO: PUT FREE() HERE
		return -1;
	}
	v = v_mem+max;

	// Alloc memory to save a copy of each version of Myers' "V" array
	if (!(v_hstry_mem = ld__malloc(pow(v_bytes, 2)))) {

		// TODO: PUT FREE() HERE
		return -1;
	}
	v_hstry = v_hstry_mem;

	v[1] = 0;  // IMPORTANT -- V's seed value

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

			// Skip over the diagonals, if any; stay inside array bounds
			// TODO: TEST BUG FIX. We added x >= 0 && y >= 0 and changed
			// x <= m && y <= n to x < m && y < n. This is to eliminate
			// access of uninitialized variables, but may compromise
			// correctness, we need to test to be sure
			while (x >= 0 && y >= 0 && x < m && y < n &&
			       rcrds1[x].hash == rcrds2[y].hash) {
				x++; y++;
			}

			// Record current endpoint for current k line
			v[k] = x;

			memcpy(v_hstry, v_mem, v_bytes);

			v_hstry += v_size;

			// Greedily terminate if we've found a path that works
			if (x >= m && y >= n) {
				printf("RESULT: %d\n", d);
				build_script(v_hstry - v_size + max, v_size, env, d, m, n, k);

				//edits(env->ses_head);

				// TODO: DECIDE WHETHER OR NOT WE WANT TO POSSIBLY SAVE THIS DATA
				// FOR SEOMTHING ELSE???
				ld__free(v_mem);
				ld__free(v_hstry_mem);

				return 0;
			}

		}
	}

	// TODO: DECIDE WHETHER OR NOT WE WANT TO POSSIBLY SAVE THIS DATA
	// FOR SEOMTHING ELSE???
	ld__free(v_mem);
	ld__free(v_hstry_mem);

	// Shouldn't happen
	return -1;
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

	free_env(&env);

	return 0;
}

