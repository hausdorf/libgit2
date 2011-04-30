/*
 * memblock.h - libdiff's internal memory block allocator.
 * Used for things like hash tables
 */
#ifndef INCLUDE_memblock_h__
#define INCLUDE_memblock_h__



typedef struct memblock {
	size_t num_units;
	size_t unit_size;
} memblock;



int memblock_init(memblock *mem, size_t num_units, size_t unit_size);


#endif /* INCLUDE memstore_h__ */
