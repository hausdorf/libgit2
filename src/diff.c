#include "diff.h"
#include "hash.h"
#include "refs.h"
#include "tree.h"
#include "commit.h"
#include "libdiff/libdiff.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* 0 on success, error on failure. The char* file_path must be free'd by
 * the caller or a memory leak will occur. The file contesnts are loaded
 * into the buffer, and the size of the buffer is loaded into size */
static int load_file(const char *file_path, char **buffer, int *size)
{
	FILE *file;
	int read_result;

	file = fopen(file_path, "rb");
	if(!file)
		return GIT_EINVALIDPATH;

	/* Get the size of this file */
	fseek(file, 0, SEEK_END);
	*size=ftell(file);
	fseek(file, 0, SEEK_SET);

	*buffer = git__malloc(*size+1);
	if (*buffer == NULL) {
		fclose(file);
		return GIT_ENOMEM;
	}

	read_result = fread(*buffer, 1, *size, file);
	if(read_result != *size) {
		free(*buffer);
		*buffer = NULL;
		fclose(file);
		return GIT_ERROR;
	}

	return GIT_SUCCESS;
}

int git_diff_no_index(git_diffresults_conf **results_conf,
		const char *filepath1, const char *filepath2)
{
	char *buffer1, *buffer2;
	int buffer1_size, buffer2_size;
	int result;

	/* verify and initialize variables */
	assert(filepath1 && filepath2);
	buffer1 = NULL;
	buffer2 = NULL;

	result = load_file(filepath1, &buffer1, &buffer1_size);
	if(result < GIT_SUCCESS)
		goto cleanup;

	result = load_file(filepath2, &buffer2, &buffer2_size);
	if(result < GIT_SUCCESS)
		goto cleanup;

	/*
	diff(NULL, NULL, NULL);
	*/

cleanup:
	if(buffer1)
		free(buffer1);
	if(buffer2)
		free(buffer2);

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

/* Prone to change, maybe easier to pass repo in then char* location */
static int file_exists(char *filename)
{
	/* TODO */
	return 0;
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
static int get_filepath(char** results, git_repository *repo,
		const git_tree_entry *entry)
{
	*results = git__malloc(sizeof(char) *
		   (strlen(git_repository_workdir(repo))
		   + strlen(git_tree_entry_name(entry))));

	if(results == NULL)
		return GIT_ENOMEM;

	strcat(*results, git_repository_workdir(repo));
	strcat(*results, git_tree_entry_name(entry));

	return GIT_SUCCESS;
}

/* Gets the changes between the git_tree_entry and the local filesystem, and
 * saves them into **results_conf. Returns 0 on success, error otherwise */
int get_file_changes(const git_tree_entry *entry, git_repository *repo,
		git_diffresults_conf **results_conf)
{
	char *filepath;		/* Filepath to a file in the working directory*/
	char *file_buffer;	/* Buffer for contents of a file */
	int file_size;		/* The size of the file in the file_buffer */
	int error;			/* Holds error results of function calls */

	/* Get the filepath from the entry */
	error = get_filepath(&filepath, repo, entry);
	if(error < GIT_SUCCESS)
		return error;

	/* If the local file exists load it into memory and compare it with the
	 * blob. If it doesn't exist, the file has been deleted from the filesystem
	 * since this entry's commit */
	if(file_exists(filepath)) {
		error = load_file(filepath, &file_buffer, &file_size);
		if(error < GIT_SUCCESS) {
			free(filepath);
			return error;
		}

		/* Check if the local file matches the git blob */
		if(!compare_hashes(file_buffer, git_tree_entry_id(entry), file_size))
			diff(NULL, NULL, NULL);

		free(file_buffer);
	}
	else {
	}

	free(filepath);
	return GIT_SUCCESS;
}

int git_diff(git_diffresults_conf **results_conf, git_commit *commit,
		git_repository *repo)
{
	git_tree *tree;				/* The tree that we will be diffing */
	const git_tree_entry *entry;/* Enteries in the tree that we are diffing */
	int error;					/* Return results of helper functions */
	unsigned int i;				/* Loop counter */

	/* Get the tree we will be diffing */
	error = get_git_tree(&tree, repo, commit);
	if(error < GIT_SUCCESS)
		return error;

	/* Compare the blobs in this tree with the files in the local filesystem */
	for(i=0; i<git_tree_entrycount(tree); i++) {
		entry = git_tree_entry_byindex(tree, i);

		error = get_file_changes(entry, repo, results_conf);
		if(error < GIT_SUCCESS) {
			git_tree_close(tree);
			return error;
		}
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
 * diff will be reveresed, ie the added stuff will show up as deleted and
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

		/* Entry1 is guarenteed to exist, Entry2 may or may not exist */
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

		/* Entry2 is guarenteed to exist, Entry1 may or may not exist */
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
