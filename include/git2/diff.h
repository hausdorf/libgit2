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
#ifndef INCLUDE_git_diff_h__
#define INCLUDE_git_diff_h__

#include "common.h"
#include "types.h"
#include "oid.h"

/**
 * @file git2/diff.h
 * @brief Git diff management routines
 * @defgroup git_repository Git diff management routines
 * @ingroup Git
 * @{
 */
GIT_BEGIN_DECL

/**
 * Diffs two files
 *
 * Diff two files such that the diff output generates the changes
 * required to change the first file into the second file. Dumps the
 * data into the diffdata param.
 *
 * Both 'file_path1' and 'file_path2' must point to some sort of file,
 * i.e., not a directory. This function will automatically detect
 * whether either 'file_path1' or 'file_path2' are actual files or not.
 *
 * @param diffdata The results of our diff operation
 * @param file_path1 The path to the first file
 * @param file_path2 The path to the second file
 *
 * @return 0 on success; error code otherwise
 */
GIT_EXTERN(int) git_diff_no_index(git_diffresults_conf **results_conf,
		const char *file_path1, const char *file_path2);

/**
 * The "standard" diff: diffs a commit (usually HEAD) and working dir
 *
 * The "standard" diff one gets when they type "git diff" into a
 * terminal somewhere. Dumps the changeset required to get from given
 * commit to the current state of the working directory into the diffdata
 * param.
 *
 * NOTE: the commit parameter is entirely optional; if you leave it empty,
 * it defaults to HEAD and you get the equivalent of the familiar
 * "git diff" command.
 *
 * @param diffdata The results of our diff operation
 * @param commit The commit to diff against working directory; if you leave
 *		it NULL, this defaults to HEAD (which will give you the equivalent
 *		of a regular "git diff")
 * @param repo The repository we're working with
 *
 * @return 0 on success; error code otherwise
 */
GIT_EXTERN(int) git_diff(git_diffresults_conf **results_conf,
		git_commit *commit, git_repository *repo);

/**
 * Equivalent to git diff --cached: diffs a commit (usually HEAD) and index
 *
 * The equivalent of "git diff --cached", this will produce a diff of the
 * whatever you have staged for commit and another commit. Dumps the
 * changeset required to get from this commit to the staged commits into
 * the diffdata param.
 *
 * NOTE: the commit parameter is entirely optional; if you leave it empty,
 * it defaults to HEAD and you get the equivalent of the familiar
 * "git diff --cached" command.
 *
 * @param diffdata The results of our diff operation
 * @param commit The commit to diff against the index; if you leave it
 *		NULL, this defaults to HEAD (which will give you the
 *		equivalent of a regular "git diff --cached")
 * @param index The index we're working with
 *
 * @return 0 on success; error code otherwise
 */
GIT_EXTERN(int) git_diff_cached(git_diffresults_conf **results_conf,
		git_commit *commit, git_index *index);

/**
 * Diffs between two arbitrary commits
 *
 * Generates the changeset required to change commit1 into commit2.
 * Dumps the diff data into diffdata param.
 *
 * @param diffdata The results of our diff operation
 * @param commit1 The first commit; we diff commit2 against it
 * @param commit2 The second commit; we diff it against commit1
 *
 * @return 0 on success; error code otherwise
 */
GIT_EXTERN(int) git_diff_commits(git_diffresults_conf **results_conf,
		git_commit *commit1, git_commit *commit2);

/** @} */
GIT_END_DECL
#endif
