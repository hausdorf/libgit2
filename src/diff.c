#include <stdio.h>
#include "diff.h"

static int load_file(char *file_name, char *buffer)
{
    FILE *file;
    file = fopen(file_name, "rb");

    if(!file)
        return appropiate_error;

    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);

    /* Allocate memory */
    buffer=(char *)malloc(fileLen+1);
    if (!buffer)
        return GIT_ENOMEM;

    /* Read file contents into buffer */
    fread(buffer, fileLen, 1, file);
    fclose(file);

    return GIT_SUCCESS;
}

int git_diff_no_index (git_diff *diff, char *file1, char *file2)
{
    char *buffer1 = NULL;
    char *buffer2 = NULL;

    /* Insure all paramater are valid */
    if(!file1 | !file2)
        return appropiate_error;

    /* load file1 into a cstring, return errof if this doesn't work */
    if(!load_file(file1, buffer1))
        return appropiate_error;

    /* load file2 into a cstring, return errof if this doesn't work */
    if(!load_file(file2, buffer2)) {
        free(buffer1);
        return appropiate_error;
    }

    /* call diff on file1, file2 */

    /* save the results indo diff_t */

    /* return git success */
    free(buffer1);
    free(buffer2);
    return GIT_SUCCESS;
}

int git_diff(git_diff *diff, git_repository *repo)
{
    /* Insure all paramaters are valid */

    /* For each file on the local filesystem, get that file out of the
     * git_repository and diff them */

    /* Return git success */
}
