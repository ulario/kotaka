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
	ACCESS_CHECK(GAME());

	relations[obj] = relation;

	obj->coset_relation(relation);
}

void coset_relation(object obj, int relation)
{
	ACCESS_CHECK(GAME());

	if (relation == 1 || relation == 3) {
		relation ^= 2;
	}

	relations[obj] = relation;
}

void clear_relation(object obj)
{
	ACCESS_CHECK(GAME());

	relations[obj] = nil;

	obj->coclear_relation(this_object());
}

void coclear_relation(object obj)
{
	ACCESS_CHECK(GAME());

	relations[obj] = nil;
}

private int relation(int ll, int lh, int rl, int rh)
{
	int i, o;

	if (lh < rl || rh < ll) {
		return 0;
	}

	if (lh >= rh && ll <= rl) {
		i = 1;
	}

	if (lh <= rh && ll >= rl) {
		o = 1;
	}

	if (i && o) {
		return 4;
	}
	if (i) {
		return 1;
	}
	if (o) {
		return 3;
	}
	return 2;
}

private int combine_relation(int a, int b)
{
	if (a == 0 || b == 0) {
		return 0;
	}

	switch(a) {
	case 1:
		if (b == 3) {
			return 0;
		}
		return b;
	case 2:
		return 2;
	case 3:
		if (b == 1) {
			return 0;
		}
		return b;
	case 4:
		return b;
	}
}

/* warning, dirty trick ahead: */
/* we deliberately have points as inverse boxes */
/* to make them always inside the box */
private int *lhof(int p, int s)
{
	if (s) {
		return ({ p, p + s });
	} else {
		return ({ p + 1, p });
	}
}

int compare_geometry(object obj)
{
	int px, py, pz;
	int lsx, lsy;
	int rsx, rsy;

	int xrel;
	int yrel;

	int ll, lh, rl, rh;

	({ px, py, pz }) = GAME_SUBD->query_position_difference(this_object(), obj);

	lsx = query_x_size();
	lsy = query_y_size();
	rsx = obj->query_x_size();
	rsy = obj->query_x_size();

	({ ll, lh }) = lhof(0, lsx);
	({ rl, rh }) = lhof(px, rsx);

	xrel = relation(ll, lh, rl, rh);

	if (!xrel) {
		return 0;
	}

	({ ll, lh }) = lhof(0, lsy);
	({ rl, rh }) = lhof(py, rsy);

	yrel = relation(ll, lh, rl, rh);

	if (!yrel) {
		return 0;
	}

	return combine_relation(xrel, yrel);
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
