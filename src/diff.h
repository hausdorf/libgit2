#ifndef INCLUDE_diff_h__
#define INCLUDE_diff_h__

#include "git2/diff.h"

enum diff_operation {
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

enum diff_handler {

	// Cause diff to be printed to stdout
	PRINT_DIFF,

	// Cause diff result data to be emitted to a custom function
	EMIT_DIFF
};



struct git_diff_conf {
	enum diff_type type;
	enum diff_handler hndlr;
};



#endif
