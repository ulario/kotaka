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
#include <kotaka/privilege.h>

object *query_inventory();
object query_environment();

int query_x_position();
int query_y_position();

int query_x_size();
int query_y_size();

mixed query_property(string key);
mixed query_local_property(string key);
void set_local_property(string key, mixed value);

mapping relations;

/* 1: insider */
/* 2: intersect */
/* 3: outsider */
/* 4: stack */

static void create()
{
	relations = ([ ]);
}

void set_relation(object obj, int relation)
{
	int corelation;
	PERMISSION_CHECK(query_environment() == obj->query_environment());

	relations[obj] = relation;

	switch(relation) {
	case 1:
	case 3:
		corelation = relation ^ 2;
		break;

	case 2:
	case 4:
		corelation = relation;
		break;

	}

	obj->coset_relation(this_object(), corelation);
}

void clear_relation(object obj)
{
	relations[obj] = nil;

	obj->coclear_relation(this_object());
}

void coset_relation(object obj, int relation)
{
	ACCESS_CHECK(GAME());

	relations[obj] = relation;
}

void coclear_relation(object obj)
{
	ACCESS_CHECK(GAME());

	relations[obj] = nil;
}

private void compare_geometry(object obj)
{
	int lxb, lyb, rxb, ryb;
	int lxe, lye, rxe, rye;
	int is_inside;
	int is_outside;

	/* if an object has a definite size, it is aligned on the gridlines */
	/* otherwise, it is within a grid cell */

	/* simulate this by doubling coordinates, and then add one if size is zero */

	int lsx, lsy, rsx, rsy;

	lxb = query_x_position() * 2;
	lyb = query_y_position() * 2;

	lsx = query_x_size() * 2;
	lsy = query_y_size() * 2;

	if (!lsx) {
		lxb++;
	}
	if (!lsy) {
		lyb++;
	}

	lxe = lxb + lsx;
	lye = lyb + lsy;

	rxb = obj->query_x_position() * 2;
	ryb = obj->query_y_position() * 2;

	rsx = obj->query_x_size() * 2;
	rsy = obj->query_y_size() * 2;

	if (!rsx) {
		rxb++;
	}
	if (!rsy) {
		ryb++;
	}

	rxe = rxb + rsx;
	rye = ryb + rsy;

	if (lxb > rxe || lyb > rye || lxe < rxb || lye < ryb) {
		clear_relation(obj);
		return;
	}

	if (lxb >= rxb && lyb >= ryb && lxe <= rxe && lye <= rye) {
		is_inside = 1;
	}

	if (lxb <= rxb && lyb <= ryb && lxe >= rxe && lye >= rye) {
		is_outside = 1;
	}

	if (is_inside) {
		if (is_outside) {
			set_relation(obj, 4);
		} else {
			set_relation(obj, 1);
		}
	} else {
		if (is_outside) {
			set_relation(obj, 3);
		} else {
			set_relation(obj, 2);
		}
	}
}

void check_geometry()
{
	object env;
	object *siblings;

	int i, sz;

	env = query_environment();

	if (!env) {
		return;
	}

	siblings = env->query_inventory();

	siblings -= ({ this_object() });

	sz = sizeof(siblings);

	for (i = 0; i < sz; i++) {
		compare_geometry(siblings[i]);
	}
}

static void move_notify(object old_env)
{
	int i, sz;
	object *ex_neighbors;

	if (!old_env) {
		return;
	}

	ex_neighbors = map_indices(relations);
	sz = sizeof(ex_neighbors);

	for (i = 0; i < sz; i++) {
		clear_relation(ex_neighbors[i]);
	}

	check_geometry();
}

nomask void touch_geometry()
{
	if (!relations) {
		relations = ([ ]);
	}

	check_geometry();
}

mapping query_relations()
{
	return relations[..];
}

int query_x_size()
{
	mixed sx;

	sx = query_local_property("size_x");

	return sx ? sx : 0;
}

int query_y_size()
{
	mixed sy;

	sy = query_local_property("size_y");

	return sy ? sy : 0;
}

void set_x_size(int sx)
{
	set_local_property("size_x", sx ? sx : nil);

	check_geometry();
}

void set_y_size(int sy)
{
	set_local_property("size_y", sy ? sy : nil);

	check_geometry();
}
