#include "diffhelpers.h"

// Taken DIRECTLY from xdiff/xutils.c
#define XDL_GUESS_NLINES 256

/**
 * FUN FACT: This function finds the number of bitshifts required to generate the
 * smallest power of two that is greater than either its size OR the bits in a
 * char * the bytes in an int.
 */
// Taken DIRECTLY from xdiff/xutils.c
unsigned int hashbits(unsigned int size)
{
	unsigned int val = 1, bits = 0;

	// WHY THE FUCK ARE WE RECOMPUTING CHAR_BIT * sizeof(unsigned int) EVERY TIME???
	for (; val < size && bits < CHAR_BIT * sizeof(unsigned int); val <<= 1, bits++);
	return bits ? bits: 1;
}

// Taken DIRECTLY from xdiff/xutils.c
// TODO: is this method necessary? Should we just call
// the size member?
long diff_mem_size(diff_mem_data *mmf)
{
	return mmf->size;
}

// Taken DIRECTLY from xdiff/xutils.c
void *diff_mem_next(diff_mem_data *mmf, long *size)
{
	return NULL;
}

// Taken DIRECTLY form xdiff/xutils.c
void *diff_mem_first(diff_mem_data *mmf, long *size)
{
	*size = mmf->size;
	return mmf->data;
}

// Taken DIRECTLY from xdiff/xutils.c
long guess_lines(diff_mem_data *mf)
{
	long nl = 0, size, tsize = 0;
	char const *data, *cur, *top;

	if ((cur = data = diff_mem_first(mf, &size)) != NULL) {
		for (top = data + size; nl < XDL_GUESS_NLINES;) {
			if (cur >= top) {
				tsize += (long) (cur - data);
				if (!(cur = data = diff_mem_next(mf, &size)))
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
		nl = diff_mem_size(mf) / (tsize / nl);

	return nl + 1;

}

