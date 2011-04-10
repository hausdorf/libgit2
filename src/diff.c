#include "diff.h"

typedef struct {
	// begin..end of sequences a, b
	const long *begin_a, *end_a, *begin_b, *end_b;
	// the difference value of begin b, a
	const long *k;
} middle_edit;

