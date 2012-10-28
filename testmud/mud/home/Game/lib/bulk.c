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
#include <kotaka/assert.h>
#include <game/paths.h>

inherit LIB_OBJECT;

/* keep it short and sweet and to the point for now */
float mass;		/* kg */

/* caching */
int bulk_dirty;			/* if cache is invalid */
int bulk_queued;		/* if we are in the bulkd queue */
float cached_content_mass;	/* cached mass of our contents */

void bulk_invalidate(varargs int force);	/* invalidate */
void bulk_sync(varargs int force);		/* synchronize bulk cache */

static void create()
{
	mass = 0.0;
}

/* kilograms */
void set_mass(float new_mass)
{
	object env;

	if (new_mass < 0.0) {
		error("Negative mass");
	}

	if (new_mass == mass) {
		return;
	}

	if (env = query_environment()) {
		env->bulk_invalidate();

		if (!env->query_bulk_queued()) {
			env->bulk_queue();
		}
	}

	mass = new_mass;
}

/* kilograms */
float query_mass()
{
	return mass;
}

/* mass of us + our contents */
float query_total_mass()
{
	if (bulk_dirty) {
		bulk_sync();
	}

	return mass + cached_content_mass;
}

/***********/
/* Caching */
/***********/

int query_bulk_dirty()
{
	return bulk_dirty;
}

int query_bulk_queued()
{
	return bulk_queued;
}

void bulk_sync(varargs int force)
{
	int i;
	int sz;
	object *inv;
	float sum;

	if (!bulk_dirty && !force) {
		return;
	}

	inv = query_inventory();
	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		sum += inv[i]->query_total_mass();
	}

	cached_content_mass = sum;
	bulk_dirty = 0;
}

void bulk_invalidate(varargs int force)
{
	object env;

	if (bulk_dirty && !force) {
		/* if we're already dirty, our containers should be too */
		return;
	}

	bulk_dirty = 1;

	if (env = query_environment()) {
		env->bulk_invalidate(force);
	}
}

void bulk_dequeued()
{
	bulk_queued = 0;
}

void bulk_queue(varargs int force)
{
	if (!force) {
		ASSERT(bulk_dirty);
		ASSERT(!bulk_queued);
	}

	bulk_queued = 1;

	BULKD->enqueue(this_object());
}

/* hooks */

static void move_notify(object old_env)
{
	object env;

	if (!mass && !bulk_dirty && !cached_content_mass) {
		return;
	}

	if (env = query_environment()) {
		env->bulk_invalidate(); /* recursive */

		if (!env->query_bulk_queued()) {
			env->bulk_queue();
		}
	}

	if (old_env) {
		old_env->bulk_invalidate(); /* recursive */

		if (!old_env->query_bulk_queued()) {
			old_env->bulk_queue();
		}
	}
}
