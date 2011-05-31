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
#include "environment.h"



int init_diff_env(struct diff_env *env, struct diff_mem *diffme1,
		struct diff_mem *diffme2)
{
	env->diffme1 = diffme1;
	env->diffme2 = diffme2;

	return 0;
}


void free_edt_scrpt(struct edit *script)
{
	ld__free(script);
}


void free_rcrds(struct record *r)
{
	ld__free(r);
}


void free_env(struct diff_env *env)
{
	free_rcrds(env->rcrds1);
	free_rcrds(env->rcrds2);
	free_edt_scrpt(env->ses_mem);
}

