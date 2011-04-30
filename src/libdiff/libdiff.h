#ifndef INCLUDE_libdiff_h__
#define INCLUDE_libdiff_h__

#include "../common.h"



struct diff_mem {
	char *data;
	size_t size;
};



int diff(struct diff_mem *diffme1, struct diff_mem *diffme2);



#endif /* INCLUDE_libdiff_h__ */
