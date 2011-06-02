/*
 * libdiff.h - STUFF FOR EXPORT: structs, macros, functions, etc
 * that are useful or required for things that use libdiff.
 */
#ifndef INCLUDE_libdiff_h__
#define INCLUDE_libdiff_h__

#include "../common.h"

#define END_OF_SCRIPT 0
#define INSERTION 1
#define DELETION 2



struct diff_mem {
	char *data;
	size_t size;
};


struct edit {
	struct record *rcrd;
	struct edit *next;
	unsigned char edit;
	size_t x;
	size_t y;
	size_t k;
};


struct record {
	unsigned long start;
	unsigned long end;
	unsigned long hash;
};



int diff(struct diff_mem *diffme1, struct diff_mem *diffme2);



#endif /* INCLUDE_libdiff_h__ */
