#ifndef INCLUDE_memstore_h__
#define INCLUDE_memstore_h__

#include "../common.h"
#include "difftypes.h"

int memstore_init(memstore *mem, long unit_size, long unit_count);
void memstore_alloc(memstore *mem);
void memstore_free(memstore *mem);

#endif

