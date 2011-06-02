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



// Holds data we're diffing
struct diff_mem {
	char *data;
	size_t size;
};


// An array of these makes up an edit script; basically they tell
// you the steps required to transform one set of content into
// another, e.g., ab -> b via DELETION of the first 'a'.
struct edit {

	// OPTIONAL: The record to insert is included if INSERTION
	struct record *rcrd;
	// OPTIONAL: The next edit in the script; NULL if last edit
	struct edit *next;
	// Type of edit -- INSERTION, DELETION, or END_OF_SCRIPT
	unsigned char edit;
	// Var names _directly_ from the Myers paper
	size_t x;
	size_t y;
	size_t k;
};



// Corresponds to lines in human-readable content
struct record {

	// Where in a diff_mem instance the record exists; in
	// human-readable content, this usually is a span of chars,
	// e.g., from diff_mem->data[12] to diff_mem->data[27].
	unsigned long start;
	unsigned long end;
	// The hash of the record; used for quick comparisons
	unsigned long hash;
};



int diff(struct diff_mem *diffme1, struct diff_mem *diffme2);



#endif /* INCLUDE_libdiff_h__ */
