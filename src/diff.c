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
		const char *filename2)
{
    return 0;
}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff(git_diffdata **diffdata, git_commit *commit,
		git_repository *repo)
{
    return 0;
}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff_cached(git_diffdata **diffdata, git_commit *commit,
		git_index *index)
{
    return 0;
}

/* TODO TODO TODO: THIS NEEDS TO BE IMPLEMENTED */
int git_diff_commits(git_diffdata **diffdata, git_commit *commit1,
		git_commit *commit2)
{
    return 0;
}

/*!
 * Represents file data (binary or text) in memory.
 * Where the data variable points to user data and size is the size of that data
 * This is used to hold the data from one diff file.
 */
typedef struct git_diff_m_data {
	long size;
	char *data;
} git_diff_m_data;

/*!
 * Represents the memory buffer
 * Where the data variable points to user data and size is the size of that data
 * In general, this is used for binary files
 */
typedef struct git_diff_m_buffer {
	long size;
	char *data;
} git_diff_m_buffer;


