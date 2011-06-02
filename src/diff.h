#ifndef INCLUDE_diff_h__
#define INCLUDE_diff_h__

#include "git2/diff.h"

// "STANDARD" diff command
// diffs working directory against index
#define DIFF_IDX_WD 0

// git diff <commit>
// diffs working directory against <commit>
#define DIFF_WD_CMT 1

// git diff --cached
// diffs index against the HEAD commit
#define DIFF_IDX_HEAD 2

// git diff --cached <commit>
// diffs index against <commit>
#define DIFF_IDX_CMT 3

// git diff <commit> <commit>
// diffs second commit against first commit
#define DIFF_CMT_CMT 4

// git diff <commit>...<commit>
// tricky: diffs between the commit that both commits share as
// an ancestor to the second commit
#define DIFF_CMT_TO_CMT 5



struct git_diff_conf {
	unsigned char diff_type;
};



#endif
