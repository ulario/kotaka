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
float density;		/* kg/l */

/* caching */
int bulk_dirty;			/* if cache is invalid */
int bulk_queued;		/* if we are in the bulkd queue */

float cached_content_mass;	/* cached mass of our contents */
float cached_content_volume;	/* cached volume of our contents */

void bulk_invalidate(varargs int force);	/* invalidate */
void bulk_sync(varargs int force);		/* synchronize bulk cache */

static void create()
{
	mass = 0.0;
	density = 1.0;
}

/* kilograms */
void set_mass(float new_mass)
{
	object env;

	if (new_mass == mass) {
		return;
	}

	if (new_mass < 0.0) {
		error("Invalid mass");
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

float query_total_mass()
{
	if (bulk_dirty) {
		bulk_sync();
	}

	return mass + cached_content_mass;
}

void set_density(float new_density)
{
	object env;

	if (new_density == density) {
		return;
	}

	if (new_density <= 0.0) {
		error("Invalid density");
	}

	if (env = query_environment()) {
		env->bulk_invalidate();

		if (!env->query_bulk_queued()) {
			env->bulk_queue();
		}
	}

	density = new_density;
}

float query_density()
{
	return density;
}

float query_volume()
{
	if (mass) {
		return mass / density / 1000.0;
	} else {
		return -1;
	}
}

float query_total_volume()
{
	if (bulk_dirty) {
		bulk_sync();
	}

	return query_volume() + cached_content_volume;
}

float query_average_density()
{
	return query_total_mass() / query_total_volume() * 1000.0;
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
	float mass_sum;
	float volume_sum;

	if (!bulk_dirty && !force) {
		return;
	}

	inv = query_inventory();
	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		mass_sum += inv[i]->query_total_mass();
		volume_sum += inv[i]->query_total_volume();
	}

	cached_content_mass = mass_sum;
	cached_content_volume = volume_sum;

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
