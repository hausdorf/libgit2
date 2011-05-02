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
#include "include.h"
#include "libdiff.h"
#include "environment.h"



int myers(struct diff_env *env)
{
	char *s1 = "abcabba";
	char *s2 = "cbabac";

	int m = 7, n = 6;
	int max = m + n;

	int *mem = malloc(sizeof(int) * (max*2+1));
	memset(mem, 0, sizeof(int) * (max*2+1));
	int *v = mem+max;
	int x, y;

	int d, k;
	for (d = 0; d <= max; d++)
	{
		for (k = -d; k <= d; k += 2)
		{
			// Determine whether we're right or down in the edit graph
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

			// Greedily terminate if we've found a path that works
			if (x >= m && y >= n) {
				printf("RESULT: %d\n", d);
				return 0;
			}
		}
	}

	return 0;
}


int prepare_and_myers(struct diff_env *env)
{
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

