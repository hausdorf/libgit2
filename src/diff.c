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
#include <stdio.h>
#include <string.h>

typedef struct {
	/* begin..end of sequences a, b */
	const long *begin_a, *end_a, *begin_b, *end_b;
	/* the difference value of begin b, a */
	const long *k;
} middle_edit;


static int load_file(char *file_path, char *buffer, int *size)
{
    FILE *file;
    file = fopen(file_path, "rb");

    if(!file)
        return appropiate_error;

    fseek(file, 0, SEEK_END);
    size=ftell(file);
    fseek(file, 0, SEEK_SET);

    /* Allocate memory */
    buffer=(char *)malloc(size+1);
    if (!buffer)
        return GIT_ENOMEM;

    /* Read file contents into buffer */
    fread(buffer, size, 1, file);
    fclose(file);

    return GIT_SUCCESS;
}

int git_diff_no_index(git_diffdata **diffdata, const char *filename1,
		const char *filename2)
{
    char *buffer1 = NULL;
    char *buffer2 = NULL;
    int buffer1_size, buffer2_size;

    /* Insure all paramater are valid */
    if(!file1 | !file2)
        return appropiate_error;

    /* load file1 into a cstring, return errof if this doesn't work */
    if(!load_file(file1, buffer1, &buffer1_size))
        return appropiate_error;

    /* load file2 into a cstring, return errof if this doesn't work */
    if(!load_file(file2, buffer2, &buffer2_size)) {
        free(buffer1);
        return appropiate_error;
    }

    /* call diff on file1, file2 (don't forget to malloc diffdata) */

    /* return git success */
    free(buffer1);
    free(buffer2);
    return GIT_SUCCESS;
}

/* The git_oid is the sha1 identifier for this blob if I am not mistake,
 * therefore i think the fastest way to do this would be to take the sha1 of
 * the file on the local filesystem and compare it to thsi git_oid. */
static int compare_hashes(char *filename, git_oid *blob_id, int *result)
{
    /* This is just throwing something together and is prone to change */

    char *file;
    int file_size;
    git_oid file_id; /* The resulting SHA1 hash of the file */

    if(load_file(filename, file, &filesize) == NULL)
        return ENOMEM;

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
    return 0;
}

int git_diff(git_diffdata **diffdata, git_commit *commit, git_repository *repo)
{
    git_reference *reference;
    git_tree *tree;
    git_oid *tree_id;
    git_tree_entry *entry;
    git_hashtable *files_hash;
    char *filepath;
    char *working_dir;
    int compare_results;

    /* Get the tree for this diff, head if commit is null, else the commit
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

    /* Get the working directory from the repository
     * TODO - does this need to be freed? */
    working_dir = git_repository_workdir(repo);

    /* Set up the hash table used to track which files from both the
     * repository and local filesystem have been processed */
    files_hash = git_hashtable_alloc(11, /*string_hash_function todo */,
                 (git_hash_keyeq_ptr)strcmp);
    if(files_hash == NULL)
        return GIT_ENOMEM;

    /* Compare the blobs in this tree with the files in the local filesystem */
    for(int i=0; i<get_tree_entrycount(tree); i++) {
        entry = git_tree_entry_byindex(tree, i);

        /* Get the full filepath for this macro */
        filepath = malloc (char *) sizeof(char) * (strlen(working_dir)
               + strlen(git_tree_entry_name(entry)));
        strcat(filepath, working_dir)
        strcat(filepath, git_tree_entry_name(entry));

        /* If the file exists in the local filesystem, diff it. Otherwise it
         * has been deleted from the file system sense this commit */
        if(file_exists(filepath)) {
            if(compare_hashes(filepath, blob, &compare_results) < 0) {
                git_tree_close(tree);
                git_hashtable_free(files_hash);
                free(filepath);
                return ENOMEM;
            }

            if(!compare_results)
                diff();

            /* TODO - check differences in file attributes? */
        }
        else {
        }

        /* TODO - add this filepath to the hashtable */
        free(filepath);
    }

    /* Check every file on the local filesystem, to catch any new files that
     * may have been created sense the commit */
    for(each file in filesystem) {
        if(file doesn't exist in hashtable)
            mark this as a new file for the diff
    }

    /* Cleanup */
    git_tree_close(tree);
    git_hashtable_free(files_hash);
    return GIT_SUCCESS;
}


int git_diff_cached(git_diffdata **diffdata, git_commit *commit,
		git_index *index) {}


int git_diff_commits(git_diffdata **diffdata, git_commit *commit1,
		git_commit *commit2)
{
    git_tree *tree1, *tree2;

    /* Get the trees for this diff */
    if(git_commit_tree(&tree1, commit1) < GIT_SUCCESS)
        return approperate_error;
    if(git_commit_tree(&tree2, commit2) < GIT_SUCCESS)
        return approperate_error;

    /* Compare the blobs in these trees looking for differences
     * TODO - make sure there are only filenames in the tree
     * TODO - make sure that the every file from both tree is checked and
     *        accounted for */
    for(int i=0; i<get_tree_entrycount(tree); i++) {
    }

    /* Cleanup */
    git_tree_close(tree1);
    git_tree_close(tree2);
    return GIT_SUCCESS;
}


int xdl_recs_cmp(git_diffdata *dd1, long off1, long lim1,
		 git_diffdata *dd2, long off2, long lim2,
		 long *kvdf, long *kvdb, int need_min/*, xdalgoenv_t *xenv*/) {
	unsigned long const *ha1 = dd1->hashed_records, *ha2 = dd2->hashed_records;

	/*
	 * Shrink the box by walking through each diagonal snake (SW and NE).
	 */
	for (; off1 < lim1 && off2 < lim2 && ha1[off1] == ha2[off2]; off1++, off2++);
	for (; off1 < lim1 && off2 < lim2 && ha1[lim1 - 1] == ha2[lim2 - 1]; lim1--, lim2--);

	/*
	 * If one dimension is empty, then all records on the other one must
	 * be obviously changed.
	 */
	/*if (off1 == lim1) {
		char *rchg2 = dd2->rchg;
		long *rindex2 = dd2->rindex;

		for (; off2 < lim2; off2++)
			rchg2[rindex2[off2]] = 1;
	} else if (off2 == lim2) {
		char *rchg1 = dd1->rchg;
		long *rindex1 = dd1->rindex;

		for (; off1 < lim1; off1++)
			rchg1[rindex1[off1]] = 1;
	} else {
		xdpsplit_t spl;
		spl.i1 = spl.i2 = 0;*/

		/*
		 * Divide ...
		 */
		/*if (xdl_split(ha1, off1, lim1, ha2, off2, lim2, kvdf, kvdb,
			      need_min, &spl, xenv) < 0) {

			return -1;
		}*/

		/*
		 * ... et Impera.
		 */
		/*if (xdl_recs_cmp(dd1, off1, spl.i1, dd2, off2, spl.i2,
				 kvdf, kvdb, spl.min_lo, xenv) < 0 ||
		    xdl_recs_cmp(dd1, spl.i1, lim1, dd2, spl.i2, lim2,
				 kvdf, kvdb, spl.min_hi, xenv) < 0) {

			return -1;
		}
	}*/

	return 0;
}

/*int main()
{
	git_diff_data dd1, dd2;
	long off1, lim1, off2, lim2, kvdf, kvdb;
	int need_min;
	xdl_recs_cmp(&dd1, off1, lim1, &dd2, off2, lim2, &kvdf, &kvdb, need_min/ *, xdalgoenv_t *xenv* /);
}*/
