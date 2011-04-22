#include "diff.h"
#include "hash.h"
#include "refs.h"
#include "tree.h"
#include "commit.h"
#include "fileops.h"
#include "libdiff/libdiff.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>


int git_diff_no_index(git_diffresults_conf **results_conf,
		const char *filepath1, const char *filepath2)
{
	diff_mem_data data1, data2;
	gitfo_buf buffer1, buffer2;
	int result;

	/* Verify and initialize variables */
	assert(filepath1 && filepath2);
	buffer1.data = NULL;
	buffer2.data = NULL;

    /* Attempt to open file1 and read contents */
	result = gitfo_read_file(&buffer1, filepath1);
	if(result < GIT_SUCCESS)
		goto cleanup;

    /* Attempt to open file2 and read contents */
	result = gitfo_read_file(&buffer2, filepath2);
	if(result < GIT_SUCCESS)
		goto cleanup;

	/* Preform the diff, curently not implemented */
	/*
	data1.data = buffer1.data;
	data1.size = buffer1.len;
	data2.data = buffer2.data;
	data2.size = buffer2.len;
	diff(&data1, &data2, *results_conf);
	*/

cleanup:
	if(buffer1.data)
		gitfo_free_buf(&buffer1);
	if(buffer2.data)
		gitfo_free_buf(&buffer2);

	return result;
}

/* A git_oid is the SHA1 hash of a blob, so we will just create a git_oid from
 * the file_buffer and compare it to the blob. Returns 1 if they are the same,
 * 0 otherwise */
static int compare_hashes(char *file_buffer, const git_oid *blob_id,
		int file_size)
{
	git_oid file_id; 	/* The resulting SHA1 hash of the file */
	git_hash_buf(&file_id, (void *) file_buffer, file_size);

	return git_oid_cmp(&file_id, blob_id);
}

/* Gets the tree of this commit, or the HEAD tree if commit is NULL.
 * Returns 0 on success, error otherwise. */
static int get_git_tree(git_tree **results, git_repository *repo,
		git_commit *commit)
{
	git_reference *sym_ref;		/* Symbolic reference, used for head */
	git_reference *direct_ref;	/* Direct reference, resolved from sym_ref */
	const git_oid *oid;
	int error;

	if(!commit) {
		/* Get the reference for the repo's head */
		error = git_reference_lookup(&sym_ref, repo, "HEAD");
		if(error < GIT_SUCCESS)
			return error;

		/* Convert the symbolic reference to a direct reference so we
		 * can get the git_oid from it */
		error = git_reference_resolve(&direct_ref, sym_ref);
		if(error < GIT_SUCCESS)
			return error;

		/* Get the head commit */
		oid = git_reference_oid(direct_ref);
		git_commit_lookup(&commit, repo, oid);
	}

	return git_commit_tree(results, commit);
}

/* The resulting char* must be freed by caller or a memory leak will occur */
static int get_filepath(char** results, git_repository *repo, char *subdir,
		const git_tree_entry *entry)
{
	size_t dir_length;
	size_t file_length;
	size_t subdir_length;

	dir_length = strlen(git_repository_workdir(repo));
	file_length = strlen(git_tree_entry_name(entry));
	subdir_length = strlen(subdir);

	*results = git__malloc(dir_length + file_length + subdir_length + 1);

	if(*results == NULL)
		return GIT_ENOMEM;

	strcpy(*results, git_repository_workdir(repo));
	strcat(*results, subdir);
	strcat(*results, git_tree_entry_name(entry));

	printf("%s", *results);
	printf("\n");

	return GIT_SUCCESS;
}

/* Gets the changes between the git_tree_entry and the local filesystem, and
 * saves them into **results_conf. Returns 0 on success, error otherwise */
int get_file_changes(const git_tree_entry *entry, git_repository *repo,
		char *subdir, git_diffresults_conf **results_conf)
{
	char *filepath;				/* Path to a file in the working directory*/
	gitfo_buf buffer;			/* Buffer to hold the contects of a file */
	int error;					/* Holds error results of function calls */

	/* Verify and initialize data */
	assert(entry && repo);
	filepath = NULL;
	buffer.data = NULL;

	/* Get the file path from the entry */
	error = get_filepath(&filepath, repo, subdir, entry);
	if(error < GIT_SUCCESS)
		return error;

	/* If the file is no longer present, it has been deleted since this entries
	 * commit (NOTE this method is wierd and returns 0 if the file exists) */
    if(gitfo_exists(filepath) != 0) {
		/* TODO - Mark this file as deleted in the diff */
		goto cleanup;
    }

	/* Attempt to open the file and read its contents */
	error = gitfo_read_file(&buffer, filepath);
	if(error < GIT_SUCCESS)
		goto cleanup;

	/* Check if the local file matches the git blob, and diff it if not */
	if(!compare_hashes(buffer.data, git_tree_entry_id(entry), buffer.len))
		diff(NULL, NULL, NULL); /* TODO */

cleanup:
	if(buffer.data)
		gitfo_free_buf(&buffer);
	if(filepath)
		free(filepath);

	return GIT_SUCCESS;
}

static int get_subdir(char **new_subdir, char *subdir,
		const git_tree_entry *entry)
{
	size_t original_length;
	size_t entry_length;

	original_length = strlen(subdir);
	entry_length = strlen(git_tree_entry_name(entry));

	/* +2, one for the null character terminator, and one for an extra '/' that
	 * needs to be placed after the new subdir to make a valid filepath */
	*new_subdir = git__malloc(original_length + entry_length + 2);

	if(*new_subdir == NULL)
		return GIT_ENOMEM;

	strcpy(*new_subdir, subdir);
	strcat(*new_subdir, git_tree_entry_name(entry));
	strcat(*new_subdir, "/"); /* TODO - will this work for windows? */

	return GIT_SUCCESS;
}

/* Recursivly diffs a git_tree, navigating down every tree in this
 * entry and diffing all the blobs to the local filesystem. The subdir
 * char array is so that we can figure out what sub directory (tree) we
 * are in as we recursivly go through the trees, and build the correct
 * filepatch to the local filesystem */
int diff_tree_to_filesystem(git_diffresults_conf **results_conf,
	 	git_repository *repo, git_tree *tree, char* subdir)
{
	const git_tree_entry *entry;
	git_tree *sub_tree; /* Holder for a sub tree that this tree points to */
	char *new_subdir;
	git_object *object;
	unsigned int i;
	int error;

	/* Check every tree entry. If one points to another tree go through all
	 * of the entries in that tree. If one points to a blob diff that blob
	 * with the local file system */
	for(i=0; i<git_tree_entrycount(tree); i++) {
		entry = git_tree_entry_byindex(tree, i);
		git_tree_entry_2object(&object, repo, entry);

		if(git_object_type(object) == GIT_OBJ_TREE) {
			/* Get the sub directory and sub tree so that we can recurisly
			 * go through all the trees */
			get_subdir(&new_subdir, subdir, entry);
			git_tree_lookup(&sub_tree, repo, git_object_id(object));

			diff_tree_to_filesystem(results_conf, repo, sub_tree, new_subdir);

			free(new_subdir);
		}
		else {
			error = get_file_changes(entry, repo, subdir, results_conf);
			if(error < GIT_SUCCESS)
				return error;
		}
	}

	return GIT_SUCCESS;
}

/* Recursivly goes through this tree to all other trees this points to,
 * checing all of the blobs */
int git_diff(git_diffresults_conf **results_conf, git_commit *commit,
		git_repository *repo)
{
	git_tree *tree;				/* The tree that we will be diffing */
	int error;					/* Return results of helper functions */
	unsigned int i;				/* Loop counter */

	/* Get the tree we will be diffing */
	error = get_git_tree(&tree, repo, commit);
	if(error < GIT_SUCCESS)
		return error;

	/* Preform the diff of everything in the tree */
	error = diff_tree_to_filesystem(results_conf, repo, tree, "");
	if(error < GIT_SUCCESS) {
		git_tree_close(tree);
		return error;
	}

	/* Check every file on the local filesystem, to catch any new files that
	 * may have been created since the commit */
	/*
	for(each file in filesystem) {
		entry = git_tree_entry_byname(tree, filename);

		if(entry == NULL) {
			This is a newly created file since the commit we are diffing
		}
	}
	*/

	git_tree_close(tree);
	return GIT_SUCCESS;
}

int git_diff_cached(git_diffresults_conf **results_conf, git_commit *commit,
		git_index *index)
{
	/* TODO */
    return 0;
}

/* Assumes commit1 is newer then commit 2. If this is not the case then the
 * diff will be reversed, i.e. the added stuff will show up as deleted and
 * vice versa */
int git_diff_commits(git_diffresults_conf **results_conf, git_commit *commit1,
		git_commit *commit2)
{
	git_tree *tree1, *tree2;
	const git_tree_entry *entry1, *entry2;
	const git_oid *blob1, *blob2;
	const char *filename;
	int results;
	unsigned int i;

	results = git_commit_tree(&tree1, commit1);
	if(results < GIT_SUCCESS)
		goto cleanup;

	results = git_commit_tree(&tree2, commit2);
	if(results < GIT_SUCCESS)
		goto cleanup;

	/* Compare all the blobs in tree1 looking for differences between tree2 */
	for(i=0; i<git_tree_entrycount(tree1); i++) {
		entry1 = git_tree_entry_byindex(tree1, i);
		filename = git_tree_entry_name(entry1);

		/* Entry1 is guaranteed to exist, Entry2 may or may not exist */
		entry2 = git_tree_entry_byname(tree2, filename);

		if(!entry2) {
			/* this entry got deleted between commit1 and commit2 */
		}
		else {
			blob1 = git_tree_entry_id(entry1);
			blob2 = git_tree_entry_id(entry1);
			if(git_oid_cmp(blob1, blob2))
				diff(NULL, NULL, NULL);
		}
	}

	/* Check tree2 for files that were added between these two commits */
	for(i=0; i<git_tree_entrycount(tree2); i++) {
		entry2 = git_tree_entry_byindex(tree2, i);
		filename = git_tree_entry_name(entry2);

		/* Entry2 is guaranteed to exist, Entry1 may or may not exist */
		entry1 = git_tree_entry_byname(tree2, filename);

		if(!entry1) {
			/* this entry got added between commit1 and commit2 */
		}
	}

cleanup:
	if(tree1)
		git_tree_close(tree1);
	if(tree2)
		git_tree_close(tree2);

	return results;
}
