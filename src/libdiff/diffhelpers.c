#include "diffhelpers.h"

// Taken DIRECTLY from xdiff/xutils.c
#define XDL_GUESS_NLINES 256


// TODO: REPLACE THIS WITH THE Q3_SQRT()??
// Taken DIRECTLY form xdiff/xutils.c
long bogosqrt(long n) {
	long i;

	/*
	 * Classical integer square root approximation using shifts.
	 */
	for (i = 1; n > 0; n >>= 2)
		i <<= 1;

	return i;
}


// Taken DIRECTLY form xdiff/xutils.c
int record_match(const char *l1, long s1, const char *l2, long s2, long flags)
{
	int i1, i2;

	if (s1 == s2 && !memcmp(l1, l2, s1))
		return 1;
	if (!(flags & XDF_WHITESPACE_FLAGS))
		return 0;

	i1 = 0;
	i2 = 0;

	/*
	 * -w matches everything that matches with -b, and -b in turn
	 * matches everything that matches with --ignore-space-at-eol.
	 *
	 * Each flavor of ignoring needs different logic to skip whitespaces
	 * while we have both sides to compare.
	 */
	if (flags & XDF_IGNORE_WHITESPACE) {
		goto skip_ws;
		while (i1 < s1 && i2 < s2) {
			if (l1[i1++] != l2[i2++])
				return 0;
		skip_ws:
			while (i1 < s1 && XDL_ISSPACE(l1[i1]))
				i1++;
			while (i2 < s2 && XDL_ISSPACE(l2[i2]))
				i2++;
		}
	} else if (flags & XDF_IGNORE_WHITESPACE_CHANGE) {
		while (i1 < s1 && i2 < s2) {
			if (XDL_ISSPACE(l1[i1]) && XDL_ISSPACE(l2[i2])) {
				/* Skip matching spaces and try again */
				while (i1 < s1 && XDL_ISSPACE(l1[i1]))
					i1++;
				while (i2 < s2 && XDL_ISSPACE(l2[i2]))
					i2++;
				continue;
			}
			if (l1[i1++] != l2[i2++])
				return 0;
		}
	} else if (flags & XDF_IGNORE_WHITESPACE_AT_EOL) {
		while (i1 < s1 && i2 < s2 && l1[i1++] == l2[i2++])
			; /* keep going */
	}

	/*
	 * After running out of one side, the remaining side must have
	 * nothing but whitespace for the lines to match.  Note that
	 * ignore-whitespace-at-eol case may break out of the loop
	 * while there still are characters remaining on both lines.
	 */
	if (i1 < s1) {
		while (i1 < s1 && XDL_ISSPACE(l1[i1]))
			i1++;
		if (s1 != i1)
			return 0;
	}
	if (i2 < s2) {
		while (i2 < s2 && XDL_ISSPACE(l2[i2]))
			i2++;
		return (s2 == i2);
	}
	return 1;
}


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


/*
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

