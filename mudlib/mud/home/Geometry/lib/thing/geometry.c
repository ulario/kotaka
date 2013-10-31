/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/paths/geometry.h>
#include <kotaka/paths/thing.h>
#include <kotaka/privilege.h>

inherit LIB_THING;
inherit xyz "xyz";

object *query_inventory();
object query_environment();

mixed query_property(string key);
mixed query_local_property(string key);
void set_local_property(string key, mixed value);

private mapping relations;	/* relations to other objects */

/* 1: inside us */
/* 2: intersecting us */
/* 3: surrounding us */
/* 4: overlapping us */

static void create()
{
	relations = ([ ]);
}

void set_relation(object obj, int relation)
{
	ACCESS_CHECK(GEOMETRY());

	relations[obj] = relation;

	obj->coset_relation(this_object(), relation);
}

void coset_relation(object obj, int relation)
{
	ACCESS_CHECK(GEOMETRY());

	if (relation & 1) {
		relation ^= 2;
	}

	relations[obj] = relation;
}

void clear_relation(object obj)
{
	ACCESS_CHECK(GEOMETRY());

	relations[obj] = nil;

	obj->coclear_relation(this_object());
}

void coclear_relation(object obj)
{
	ACCESS_CHECK(GEOMETRY());

	relations[obj] = nil;
}

private void reset_relations()
{
	int sz;
	object *neighbors;

	neighbors = map_indices(relations);

	for (sz = sizeof(neighbors) - 1; sz >= 0; sz--) {
		clear_relation(neighbors[sz]);
	}
}

static int combine_relation(int a, int b)
{
	if (a == 0 || b == 0) {
		return 0;
	}

	switch(a) {
	case 1:
		if (b == 3 || b == 2) {
			return 2;
		}
		return 1;

	case 2:
		return 2;

	case 3:
		if (b == 1 || b == 2) {
			return 2;
		}
		return 3;

	case 4:
		return b;
	}
}

/* do a simple search for relations */
/* this simply checks against all our thingwise siblings */
private void update_relations_simple()
{
	object env;
	object *siblings;

	int i, sz;

	reset_relations();

	env = query_environment();

	if (!env) {
		return;
	}

	siblings = env->query_inventory();

	siblings -= ({ this_object() });

	sz = sizeof(siblings);

	for (i = 0; i < sz; i++) {
		int relation;

		relation = xyz_compare_geometry(siblings[i]);

		if (relation) {
			set_relation(siblings[i], relation);
		} else {
			clear_relation(siblings[i]);
		}
	}
}

void update_relations()
{
}

void check_geometry()
{
	update_relations_simple();
}

static void move_notify(object old_env)
{
	xyz::move_notify(old_env);

	check_geometry();
}

mapping query_relations()
{
	return relations[..];
}
