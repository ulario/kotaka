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

private string mass_derivation;

private int mass_absolute;
private int density_absolute;
private int capacity_absolute;
private int max_mass_absolute;

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
	mass = 0.0;
	density = 1.0;
}

/* patching */

void patch_bulk()
{
	if (mass_derivation == "absolute") {
		mass_absolute = 1;
		density_absolute = 1;
		capacity_absolute = 1;
		max_mass_absolute = 1;
	}
}

/* mass */

void set_local_mass(float new_mass)
{
	object env;

	if (new_mass == mass) {
		return;
	}

	if (env = query_environment()) {
		env->bulk_invalidate();
	}

	mass = new_mass;
}

float query_local_mass()
{
	return mass;
}

void set_mass(float new_mass)
{
	object arch;

	arch = query_archetype();

	if (virtual) {
		error("Cannot set mass of virtual thing");
	} else if (mass_absolute || !(arch = query_archetype())) {
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
	} else if (mass_absolute || !(arch = query_archetype())) {
		return mass;
	} else {
		return mass * arch->query_mass();
	}
}

void set_mass_absolute(int new_mass_absolute)
{
	object env;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}

	mass_absolute = new_mass_absolute;
}

int query_mass_absolute()
{
	return mass_absolute;
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
	} if (density_absolute || !(arch = query_archetype())) {
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
	} else if (density_absolute || !(arch = query_archetype())) {
		return density;
	} else {
		return density * arch->query_density();
	}
}

void set_density_absolute(int new_density_absolute)
{
	object env;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}

	density_absolute = new_density_absolute;
}

int query_density_absolute()
{
	return density_absolute;
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
	} else if (capacity_absolute || !(arch = query_archetype())) {
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
	} else if (capacity_absolute || !(arch = query_archetype())) {
		return capacity;
	} else {
		return capacity * arch->query_capacity();
	}
}

void set_capacity_absolute(int new_capacity_absolute)
{
	object env;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}

	capacity_absolute = new_capacity_absolute;
}

int query_capacity_absolute()
{
	return capacity_absolute;
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
	} if (max_mass_absolute || !(arch = query_archetype())) {
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
	} if (max_mass_absolute || !(arch = query_archetype())) {
		return max_mass;
	} else {
		return max_mass * arch->query_max_mass();
	}
}

void set_max_mass_absolute(int new_max_mass_absolute)
{
	object env;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}

	max_mass_absolute = new_max_mass_absolute;
}

int query_max_mass_absolute()
{
	return max_mass_absolute;
}

/* flexible */

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
	virtual = new_virtual;
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
		"virtual": query_virtual() ? 1 : nil,
		"mass_absolute": query_mass_absolute() ? 1 : nil,
		"density_absolute": query_density_absolute() ? 1 : nil,
		"capacity_absolute": query_capacity_absolute() ? 1 : nil,
		"max_mass_absolute": query_max_mass_absolute() ? 1 : nil,
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

	set_local_mass(v ? v : 0.0);

	set_virtual(map["virtual"] ? 1 : 0);
	set_flexible(map["flexible"] ? 1 : 0);

	set_mass_absolute(map["mass_absolute"] ? 1 : 0);
	set_density_absolute(map["density_absolute"] ? 1 : 0);
	set_capacity_absolute(map["capacity_absolute"] ? 1 : 0);
	set_max_mass_absolute(map["max_mass_absolute"] ? 1 : 0);
}
