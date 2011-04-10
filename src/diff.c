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

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff_no_index(git_diffdata **diffdata, const char *filename1,
		const char *filename2) {}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff(git_diffdata **diffdata, git_commit *commit,
		git_repository *repo) {}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff_cached(git_diffdata **diffdata, git_commit *commit,
		git_index *index) {}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff_commits(git_diffdata **diffdata, git_commit *commit1,
		git_commit *commit2) {}

/**
 * TODO: give credit to Eppstein, as this is an implementation of his Memoized LCS
 * http://www.ics.uci.edu/~eppstein/161/960229.html
 */
int lcs_length(git_diffdata *data_a, git_diffdata *data_b, 
{


}
