/*
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2,
 * as published by the Free Software Foundation.
 *
 * In addition to the permissions in the GNU General Public License,
 * the authors give you unlimited permission to link the compiled
 * version of this file into combinations with other programs,
 * and to distribute those combinations without any restriction
 * coming from the use of this file.  (The General Public License
 * restrictions do apply in other respects; for example, they cover
 * modification of the file, and distribution when not linked into
 * a combined executable.)
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "memstore.h"

int memstore_init(memstore *mem, long unit_size, long unit_count)
{
	mem->head = mem->tail = NULL;
	mem->unit_size = unit_size;
	mem->store_size = unit_size * unit_count;
	mem->allocator = mem->iterator = NULL;
	mem->scurr = 0;

	return 0;
}

void *memstore_alloc(memstore *mem)
{
	// So I *THINK* the basic idea here is, when you first call the function,
	// to initially malloc a ton of space, and then, every call after that,
	// we just continue to return slots in that space as necessary. So if we
	// have an xdfile that our chastore is supposed to be storing, we will
	// initially malloc as much space as we've said we need in xdl_cha_init(),
	// and then to just return addresses of blocks of the same size as xdfile
	// every time we call this method after that.
	//
	// The only exception to this case seems to be if we run out of memory;
	// then we malloc even more space, point the chanode->next at the
	// beginning of this new block, and reset the ancur->icurr = 0.
	//
	// Internally, the chastore->ancur handles the allocation of "new" blocks.
	// Really what it does is to iterate through this massive malloc'd block
	// and hand off addresses of available memory. It does this by
	// incrementing icurr by the number of bytes appropriate.
	memstore_node *allocator;
	void *data;

	// "malloc more space if cha->ancur is NULL or, failing that, if ancur->icurr
	// is as big as the chastore's size"
	//
	// First half of this if statement assigns our chanode *ancur to cha->ancur;
	// if this is NULL, we are definitely going to malloc. In the case that ancur
	// (and, by extension, cha->ancur) is NOT null, we look to see if the
	// iterator ancur->icurr points to the last available malloc'd location in
	// memory; if it does, we need to malloc more memory.
	if(!(allocator = mem->allocator) || allocator->curr_idx == mem->store_size) {
		// allocate memory equal to the entire store's memory, plus 1 chanode
		if(!(allocator = (memstore_node *) malloc(sizeof(memstore_node) + mem->store_size))) {

			return NULL;
		}
		// set index of current cha to 0
		allocator->curr_idx = 0;
		allocator->next = NULL;
		// if tail set, point current tail's next at current
		if(mem->tail)
			mem->tail->next = allocator;
		// define head if not done
		if(!mem->head)
			mem->head = allocator;
		// and then set tail to point to this new memory block
		mem->tail = allocator;
		mem->allocator = allocator;
	}

	// build address to hand back; can be casted as a pointer, because it points
	// to a new location in memory capable of holding the sort of record we
	// initialized with xdl_cha_init()
	//
	// No idea how they decided to calculate this
	data = (char *) allocator + sizeof(memstore_node) + allocator->curr_idx;
	// move the current index forward by the amount of space of the thing we're
	// measuring (e.g., move the cursor forward by sizeof(xfile_t). Must happen
	// before return
	allocator->curr_idx += mem->unit_size;

	return data;
}

void memstore_free(memstore *mem)
{
	memstore_node *cur, *tmp;

	for(cur = mem->head; (tmp = cur) != NULL;) {
		cur = cur->next;
		free(tmp);
	}
}

