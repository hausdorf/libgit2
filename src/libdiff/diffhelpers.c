#include "diffhelpers.h"

// Taken DIRECTLY from xdiff/xutils.c
#define XDL_GUESS_NLINES 256

// Taken DIRECTLY from xdiff/xutils.c
// TODO: is this method necessary? Should we just call
// the size member?
long xdl_mmfile_size(diff_mem_data *mmf)
{
	return mmf->size;
}

// Taken DIRECTLY from xdiff/xutils.c
void *xdl_mmfile_next(diff_mem_data *mmf, long *size)
{
	return NULL;
}

// Taken DIRECTLY form xdiff/xutils.c
void *xdl_mmfile_first(diff_mem_data *mmf, long *size)
{
	*size = mmf->size;
	return mmf->data;
}

// Taken DIRECTLY from xdiff/xutils.c
long guess_lines(diff_mem_data *mf)
{
	long nl = 0, size, tsize = 0;
	char const *data, *cur, *top;

	if ((cur = data = xdl_mmfile_first(mf, &size)) != NULL) {
		for (top = data + size; nl < XDL_GUESS_NLINES;) {
			if (cur >= top) {
				tsize += (long) (cur - data);
				if (!(cur = data = xdl_mmfile_next(mf, &size)))
					break;
				top = data + size;
			}
			nl++;
			if (!(cur = memchr(cur, '\n', top - cur)))
				cur = top;
			else
				cur++;
		}
		tsize += (long) (cur - data);
	}

	if (nl && tsize)
		nl = xdl_mmfile_size(mf) / (tsize / nl);

	return nl + 1;

}

