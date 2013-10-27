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

int query_x_size();
int query_y_size();

mixed query_local_property(string name);
void set_local_property(string name, mixed value);
void check_geometry();

int combine_relation(int rela, int relb);

static int relation(int ll, int lh, int rl, int rh)
{
	int i, o;

	if (ll == rl && lh == rh) {
		return 4;
	}

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

/* warning, dirty trick ahead: */
/* we deliberately have points as inverse boxes */
/* to make them always inside the box */
static int *lhof(int p, int s)
{
	if (s) {
		return ({ p, p + s });
	} else {
		return ({ p + 1, p });
	}
}

int xyz_compare_geometry(object obj)
{
	int px, py, pz;
	int lsx, lsy;
	int rsx, rsy;

	int xrel;
	int yrel;

	int ll, lh, rl, rh;

	({ px, py, pz }) = GEOMETRY_SUBD->query_position_difference(this_object(), obj);

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
