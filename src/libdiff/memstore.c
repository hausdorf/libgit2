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

void memstore_alloc(memstore *mem)
{
}

void memstore_free(memstore *mem)
{
	memstore_node *cur, *tmp;

	for(cur = mem->head; (tmp = cur) != NULL;) {
		cur = cur->next;
		free(tmp);
	}
}

