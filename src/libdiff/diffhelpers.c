#include "diffhelpers.h"

// Taken DIRECTLY from xdiff/xutils.c
#define XDL_GUESS_NLINES 256

// Taken DIRECTLY form xdiff/xutils.c
static unsigned long hash_record_with_whitespace(char const **data,
		char const *top, long flags) {
	unsigned long ha = 5381;
	char const *ptr = *data;

	for (; ptr < top && *ptr != '\n'; ptr++) {
		if (XDL_ISSPACE(*ptr)) {
			const char *ptr2 = ptr;
			int at_eol;
			while (ptr + 1 < top && XDL_ISSPACE(ptr[1])
					&& ptr[1] != '\n')
				ptr++;
			at_eol = (top <= ptr + 1 || ptr[1] == '\n');
			if (flags & XDF_IGNORE_WHITESPACE)
				; /* already handled */
			else if (flags & XDF_IGNORE_WHITESPACE_CHANGE
				 && !at_eol) {
				ha += (ha << 5);
				ha ^= (unsigned long) ' ';
			}
			else if (flags & XDF_IGNORE_WHITESPACE_AT_EOL
				 && !at_eol) {
				while (ptr2 != ptr + 1) {
					ha += (ha << 5);
					ha ^= (unsigned long) *ptr2;
					ptr2++;
				}
			}
			continue;
		}
		ha += (ha << 5);
		ha ^= (unsigned long) *ptr;
	}
	*data = ptr < top ? ptr + 1: ptr;

	return ha;
}

// Taken DIRECTLY from xdiff/xutils.c
unsigned long hash_record(char const **data, char const *top, long flags) {
	unsigned long ha = 5381;
	char const *ptr = *data;

	if (flags & XDF_WHITESPACE_FLAGS)
		return hash_record_with_whitespace(data, top, flags);

	for (; ptr < top && *ptr != '\n'; ptr++) {
		ha += (ha << 5);
		ha ^= (unsigned long) *ptr;
	}
	*data = ptr < top ? ptr + 1: ptr;

	return ha;
}

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
	// Let's not do that anymore, try the following
	unsigned int uint_bit = CHAR_BIT * sizeof(unsigned int);
	for (; val < size && bits < uint_bit; val <<= 1, bits++);
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

