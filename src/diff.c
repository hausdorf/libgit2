#include "diff.h"
#include <stdio.h>
#include <string.h>

/* 0 on success, error on failure. The char* file_path must be free'd by
 * the caller or a memory leak will occur. The file contesnts are loaded
 * into the buffer, and the size of the buffer is loaded into size */
static int load_file(char *file_path, char *buffer, int *size)
{
	FILE *file;
	int result;
	int read_result;

	result = GIT_SUCCESS;

	file = fopen(file_path, "rb");
	if(!file) {
		result = GIT_EINVALIDPATH;
		goto cleanup;
	}

	/* Get the size of this file */
	fseek(file, 0, SEEK_END);
	*size=ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer=(char *)malloc(*size+1);
	if (!buffer) {
		result = GIT_ENOMEM;
		goto cleanup;
	}

	read_result = fread(buffer, *size, 1, file);
	if(read_result != *size) {
		free(buffer);
		result = GIT_ERROR;
	}

cleanup:
	if(file)
		fclose(file);

	return result;
}

int git_diff_no_index(git_diffresults_conf **results_conf,
		const char *filename1, const char *filename2)
{
	char *buffer1, *buffer2;
	int buffer1_size, buffer2_size;
	int result;

	assert(file1 && file2);

	result = load_file(file1, buffer1, &buffer1_size);
	if(results < GIT_SUCCESS)
		goto cleanup;

	result = load_file(file2, buffer2, &buffer2_size);
	if(results < GIT_SUCCESS)
		goto cleanup;

	diff();

cleanup:
	if(buffer1)
		free(buffer1);
	if(buffer2)
		free(buffer2);

	return result;
}

/* The git_oid is the SHA1 identifier for a blob. Therefore, i think the
 * fastest way to do this would be to take the sha1 of  the file on the local
 * filesystem and compare it to this git_oid. */
static int compare_hashes(char *filename, git_oid *blob_id, int *result)
{
	char *file;
	int file_size;
	git_oid file_id; /* The resulting SHA1 hash of the file */

	if(load_file(filename, file, &file_size)  < 0)
		return GIT_ENOMEM;

	/* Compare the two git_oid */
	git_hash_buf(&file_id, (void *) file, file_size);
	if(file_id == *blob_id)
		*result = 1;
	else
		*result = 0;

	/* Cleanup */
	free(file);
	return GIT_SUCCESS;
}

/* Prone to change, maybe easier to pass repo in then char* location */
static int file_exists(char *filename, char *location)
{
	/* TODO */
	return 0;
}

/* 0 if success, error otherwise. Returns the tree of this commit, or the
 * HEAD tree if commit is NULL */
static int get_git_tree(git_tree *results, git_commit *commit)
{
	git_oid *tree_id;
	git_reference *reference;

	/* Get the tree for this diff, head if commit is null, else the commits
	 * tree */
	if(!commit) {
		if(git_reference_lookup(&reference, repo, "HEAD") < GIT_SUCCESS)
			return approperate_error;

		tree_id = git_reference_oid(head);
		if(git_tree_lookup(&tree, repo, tree_id) < GIT_SUCCESS)
			return apporperate_error;
	}
	else {
		if(git_commit_tree(&tree, commit) < GIT_SUCCESS)
			return approperate_error;
	}

	return GIT_SUCCESS;
}

/* 0 on success, error on failure. The resulting char* must be freed by the
 * caller */
static int get_filepath(char* results, git_repository *repo,
						git_tree_entry *entry)
{
		filepath = malloc (char *) sizeof(char) *
				   (strlen(git_repository_workdir(repo))
				   + strlen(git_tree_entry_name(entry)));
		if(filepath == NULL)
			return GIT_ENOMEM;

		strcat(filepath, git_repository_workdir(repo));
		strcat(filepath, git_tree_entry_name(entry));

		return GIT_SUCCESS;
}

int git_diff(git_diffresults_conf **results_conf, git_commit *commit,
		git_repository *repo)
{
	git_tree *tree;			/* The tree that we will be diffing */
	git_tree_entry *entry;  /* Enteries in the tree we are diffing */
	char *filepath;			/* Filepath to a file in the working directory*/
	int compare_results;	/* Results from the compare hash method */
	int results;			/* Return result of this function */
	int i;					/* Loop counter */

	results = GIT_SUCCESS;

	/* Get the tree we will be diffing */
	if(get_git_tree(tree, commit) < 0) {
		results = ERROR;
		goto cleanup;
	}

	/* Compare the blobs in this tree with the files in the local filesystem */
	for(i=0; i<get_tree_entrycount(tree); i++) {
		entry = git_tree_entry_byindex(tree, i);

		if(get_filepath(filepath, repo, entry) < 0) {
			results = GIT_ENOMEM;
			goto cleanup;
		}

		if(file_exists(filepath)) {
			if(compare_hashes(filepath, blob, &compare_results) < 0) {
				results = GIT_ENOMEM;
				goto cleanup;
			}

			if(!compare_results)
				diff();
		}
		else {
			/* The file has been deleted from the file system */
		}

		free(filepath);
	}

	/* Check every file on the local filesystem, to catch any new files that
	 * may have been created since the commit
	 * TODO - implement this, and make it compatible with linux and word */
	for(each file in filesystem) {
		entry = git_tree_entry_byname(tree, filename);

		if(entry == NULL) {
			/* This is a newly created file since the commit we are diffing */
		}
	}

/* Flag used with goto in case of errors to clean up the pointers */
cleanup:
	if(filepath)
		free(filepath);
	if(tree)
		git_tree_close(tree);
	if(files_hash)
		git_hashtable_free(files_hash);

	return results;
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
	git_tree_entry *entry1, *entry2;
	git_oid *blob1, *blob2;
	char *filename;
	int results;
	int i;

	results = GIT_SUCCESS;

	if(git_commit_tree(&tree1, commit1) < GIT_SUCCESS) {
		results = some_error;
		goto cleanup;
	}
	if(git_commit_tree(&tree2, commit2) < GIT_SUCCESS) {
		results = some_error;
		goto cleanup;
	}

	/* Compare all the blobs in tree1 looking for differences between tree2 */
	for(i=0; i<get_tree_entrycount(tree1); i++) {
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
			if(*blob1 != *blob2)
				diff();
		}
	}

	/* Check tree2 for files that were added between these two commits */
	for(i=0; i<get_tree_entrycount(tree2); i++) {
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

/*int main()
{
	git_diff_data dd1, dd2;
	long off1, lim1, off2, lim2, kvdf, kvdb;
	int need_min;
	xdl_recs_cmp(&dd1, off1, lim1, &dd2, off2, lim2, &kvdf, &kvdb, need_min/ *, xdalgoenv_t *xenv* /);
}*/
