/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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
#include <kotaka/paths/thing.h>

object query_archetype();
object query_environment();
object *query_inventory();

/* bulk */
private float mass;		/* kg */
private float density;		/* kg/l */

private int flexible;		/* flexible container */
private int virtual;		/* virtual container */

private float capacity;		/* m^3 */
private float max_mass;		/* kg */

/* caching */

private int bulk_dirty;			/* if cache is invalid */

private float cached_content_mass;	/* cached mass of our contents */
private float cached_content_volume;	/* cached volume of our contents */

float query_mass();
void set_mass(float new_mass);
void bulk_invalidate();			/* invalidate */
void bulk_sync(varargs int force);	/* synchronize bulk cache */

static void create()
{
	mass = 1.0;
	density = 1.0;
	capacity = 1.0;
	max_mass = 1.0;
}

/* mass */

void set_local_mass(float new_mass)
{
	object env;

	mass = new_mass;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}
}

float query_local_mass()
{
	return mass;
}

void set_mass(float new_mass)
{
	object arch;

	if (virtual) {
		error("Cannot set mass of virtual thing");
	} else if (!(arch = query_archetype())) {
		set_local_mass(new_mass);
	} else {
		set_local_mass(new_mass / arch->query_mass());
	}
}

float query_mass()
{
	object arch;

	if (virtual) {
		return 0.0;
	} else if (!(arch = query_archetype())) {
		return mass;
	} else {
		return mass * arch->query_mass();
	}
}

float query_total_mass()
{
	if (bulk_dirty) {
		bulk_sync();
	}

	return query_mass() + cached_content_mass;
}

float query_contained_mass()
{
	if (bulk_dirty) {
		bulk_sync();
	}

	return cached_content_mass;
}

void figure_mass(float volume)
{
	set_mass(volume / density * 1000.0);
}

/* density */

void set_local_density(float new_density)
{
	object env;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}

	density = new_density;
}

float query_local_density()
{
	return density;
}

void set_density(float new_density)
{
	object arch;

	if (virtual) {
		error("Cannot set density of virtual thing");
	} if (!(arch = query_archetype())) {
		set_local_density(new_density);
	} else {
		set_local_density(new_density / arch->query_density());
	}
}

float query_density()
{
	object arch;

	if (virtual) {
		return 0.0;
	} else if (!(arch = query_archetype())) {
		return density;
	} else {
		return density * arch->query_density();
	}
}

void figure_density(float volume)
{
	set_density(mass / volume / 1000.0);
}

/* volume */

float query_volume()
{
	if (virtual) {
		return 0.0;
	}

	return mass / density / 1000.0;
}

float query_total_volume()
{
	if (bulk_dirty) {
		bulk_sync();
	}

	if (flexible || virtual) {
		return query_volume() + cached_content_volume;
	} else {
		return query_volume() + capacity;
	}
}

float query_contained_volume()
{
	if (bulk_dirty) {
		bulk_sync();
	}

	return cached_content_volume;
}

/* capacity */

void set_local_capacity(float new_capacity)
{
	object env;

	capacity = new_capacity;

	/* rigid objects change size when they change capacity */
	if (!flexible && (env = query_environment())) {
		env->bulk_invalidate();
	}
}

float query_local_capacity()
{
	return capacity;
}

void set_capacity(float new_capacity)
{
	object arch;

	if (virtual) {
		error("Cannot set capacity of virtual thing");
	} else if (!(arch = query_archetype())) {
		set_local_capacity(new_capacity);
	} else {
		set_local_capacity(new_capacity / arch->query_capacity());
	}
}

float query_capacity()
{
	object arch;

	if (virtual) {
		return 0.0;
	} else if (!(arch = query_archetype())) {
		return capacity;
	} else {
		return capacity * arch->query_capacity();
	}
}

/* max mass */

void set_local_max_mass(float new_max_mass)
{
	max_mass = new_max_mass;
}

float query_local_max_mass()
{
	return max_mass;
}

void set_max_mass(float new_max_mass)
{
	object arch;

	if (virtual) {
		error("Cannot set max mass of virtual thing");
	} if (!(arch = query_archetype())) {
		set_local_max_mass(new_max_mass);
	} else {
		set_local_max_mass(new_max_mass / arch->query_max_mass());
	}
}

float query_max_mass()
{
	object arch;

	if (virtual) {
		return 0.0;
	} if (!(arch = query_archetype())) {
		return max_mass;
	} else {
		return max_mass * arch->query_max_mass();
	}
}

/* flags */

void set_flexible(int new_flexible)
{
	object env;

	flexible = new_flexible;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}
}

int query_flexible()
{
	return flexible;
}

void set_virtual(int new_virtual)
{
	object env;

	virtual = new_virtual;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}
}

int query_virtual()
{
	return virtual;
}

/***********/
/* Caching */
/***********/

int query_bulk_dirty()
{
	return bulk_dirty;
}

void bulk_sync(varargs int force)
{
	float mass_sum, volume_sum;
	int sz;
	object *inv;

	if (!bulk_dirty && !force) {
		return;
	}

	inv = query_inventory();

	for (sz = sizeof(inv); --sz >= 0; ) {
		mass_sum += inv[sz]->query_total_mass();
		volume_sum += inv[sz]->query_total_volume();
	}

	cached_content_mass = mass_sum;
	cached_content_volume = volume_sum;

	bulk_dirty = 0;
}

void bulk_invalidate()
{
	object env;

	bulk_dirty = 1;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}
}

/* save */

mapping bulk_save()
{
	mixed v;

	return ([
		"local_mass": (v = query_local_mass()) ? v : nil,
		"local_density": (v = query_local_density()) ? v : nil,
		"local_capacity": (v = query_local_capacity()) ? v : nil,
		"local_max_mass": (v = query_local_max_mass()) ? v : nil,
		"flexible": query_flexible() ? 1 : nil,
		"virtual": query_virtual() ? 1 : nil
	]);
}

void bulk_restore(mapping map)
{
	mixed v;

	v = map["local_mass"];

	if (v == nil) {
		v = map["mass"];
	}

	set_local_mass(v ? v : 0.0);

	v = map["local_density"];

	if (v == nil) {
		v = map["density"];
	}

	set_local_density(v ? v : 0.0);

	v = map["local_capacity"];

	if (v == nil) {
		v = map["capacity"];
	}

	set_local_capacity(v ? v : 0.0);

	v = map["local_max_mass"];

	if (v == nil) {
		v = map["max_mass"];
	}

	set_local_max_mass(v ? v : 0.0);

	set_virtual(map["virtual"] ? 1 : 0);
	set_flexible(map["flexible"] ? 1 : 0);
}
