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
	char *a = "abcabba";
	char *b = "cbabac";

	int n = 13, m = 17, k, d;
	int l = n+m;
	int v[n+m];
	int v_arr[100];
	char down;
	int kprev, xstart, xmid, xend, ystart, ymid, yend;
	int snake;

	memset(v, 0, sizeof(int) * (n+m));
	memset(v_arr, 0, sizeof(int) * 100);

	/*
	// list v_arr
	// snakes
	// point p

	for (d = v_arr.count - 1; p.x > 0 || p.y > 0; d--) {
		v = v_arr[d];
		k = p.x - p.y;

		// end point is in v
		xend = v[k];
		yend = x - k;

		// down or right?
		down = (k == -d || (k != d && v[k - 1] < v[k + 1]));

		kprev = down ? k + 1 : k - 1;

		// start point
		xstart = v[kprev];
		ystart = xstart - kprev;

		// mid point
		xmid = down ? xstart : xstart + 1;
		ymid = xmid - k;

		snakes.insert(0, new snake( start, mid, end points));

		p.x = xstart;
		p.y = ystart;
	}
	*/

	for (d = 0; d <= l; d++) {
		for (k = -d; k <= d; k += 2) {
			down = (k == -d || (k != d && v[k - 1] < v[k + 1]));

			kprev = down ? k + 1 : k - 1;

			xstart = v[kprev];
			printf("%d %d ", v[kprev], kprev);
			ystart = xstart - kprev;

			xmid = down ? xstart : xstart + 1;
			ymid = xmid - k;

			xend = xmid;
			yend = ymid;

			snake = 0;
			while (xend < n && yend < m && a[xend] == b[yend])
				xend++; yend++; snake++;

			v[k] = xend;

			if (xend >= n && yend >= m) {
				printf("Found solution\n\n");
				goto solutionfound;
			}
		}
	}

solutionfound:

	printf("Successfully exited loop\n");
	int i;
	for (i = 0; i < n+m+2; i++)
		printf("%d ", v[i]);

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

