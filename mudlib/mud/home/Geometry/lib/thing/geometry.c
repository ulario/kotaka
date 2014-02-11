/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013, 2014  Raymond Jennings
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

private string system;

object *query_inventory();
object query_environment();

mixed query_property(string key);
mixed query_local_property(string key);
void set_local_property(string key, mixed value);

/* 1: inside us */
/* 2: intersecting us */
/* 3: surrounding us */
/* 4: overlapping us */

static void create()
{
}

void set_coordinate_system(string type)
{
	switch(type) {
	case "cartesian":
	case nil:
		system = type;
	default:
		error("Invalid coordinate system");
	}
}

string query_coordinate_system()
{
	return system;
}

static void move_notify(object old_env)
{
	xyz::move_notify(old_env);
}
