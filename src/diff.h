#ifndef INCLUDE_diff_h__
#define INCLUDE_diff_h__

#include "git2/diff.h"

// TODO: PATIENCE HASN'T BEEN IMPLEMENTED YET
#define GIT_DIFF_PATIENCE	(1 << 0)

// Used in git_diff_setopt(), determines the option to set
enum diff_opt_type {

	DIFF_TYPE,
	DIFF_HANDLE
};

// The type of diff command we're doing (e.g., git diff --cached)
enum diff_type {

	// "STANDARD" diff command
	// diffs working directory against index
	DIFF_IDX_WD,
	// git diff <commit>
	// diffs working directory against <commit>
	DIFF_WD_CMT,
	// git diff --cached
	// diffs index against the HEAD commit
	DIFF_IDX_HEAD,
	// git diff --cached <commit>
	// diffs index against <commit>
	DIFF_IDX_CMT,
	// git diff <commit> <commit>
	// diffs second commit against first commit
	DIFF_CMT_CMT,
	// git diff <commit>...<commit>
	// tricky: diffs between the commit that both commits share as
	// an ancestor to the second commit
	DIFF_CMT_TO_CMT
};

// What we're doing with diff results (e.g., printing them)
enum diff_handle {

	// Cause diff to be printed to stdout
	PRINT_DIFF,
	// Cause diff result data to be emitted to a custom function
	EMIT_DIFF
};



struct git_diff_conf {

	// Defines the sort of diff -- e.g., diff --cached
	enum diff_type type;
	// What do do with diff results -- print, emit to function?
	enum diff_handle handle;
	// Do we look at whitespace? Are we using patience diff?
	unsigned long flags;
};



#endif
