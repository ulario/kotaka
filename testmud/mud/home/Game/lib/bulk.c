/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/checkarg.h>

inherit LIB_OBJECT;

/* keep it short and sweet and to the point for now */
float mass;		/* kg */

/* caching */
float cached_content_mass;	/* cached mass of our contents */
int dirty;

void bulk_invalidate();
void bulk_sync();

static void create()
{
	mass = 0.0;
}

/* kilograms */
void set_mass(float new_mass)
{
	if (new_mass < 0.0) {
		error("Negative mass");
	}

	mass = new_mass;
	bulk_invalidate();
}

/* kilograms */
float query_mass()
{
	return mass;
}

/* mass of us + our contents */
float query_total_mass()
{
	ACCESS_CHECK(GAME());

	if (dirty) {
		bulk_sync();
	}

	return mass + cached_content_mass;
}

/***********/
/* Caching */
/***********/

void bulk_sync(varargs int force)
{
	int i;
	int sz;
	object *inv;
	float sum;

	ACCESS_CHECK(GAME());

	if (!dirty && !force) {
		return;
	}

	inv = query_inventory();
	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		object subobj;

		subobj = inv[i];

		subobj->bulk_sync(force);
		sum += subobj->query_total_mass();
	}

	cached_content_mass = sum;
	dirty = 0;
}

void bulk_invalidate()
{
	object env;

	ACCESS_CHECK(GAME());

	dirty = 1;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}
}
