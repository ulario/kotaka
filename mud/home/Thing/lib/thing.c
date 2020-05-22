/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020  Raymond Jennings
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
inherit arch "thing/archetype";
inherit id "thing/id";
inherit inv "thing/inventory";
inherit prop "thing/property";
inherit bulk "thing/bulk";

/*****************/
/* General stuff */
/*****************/

static void create()
{
	arch::create();
	id::create();
	inv::create();
	prop::create();
	bulk::create();
}

static void destruct()
{
	object env;

	env = query_environment();

	if (env) {
		env->bulk_invalidate();
	}
}

static mapping save()
{
	mixed v;

	return ([
		"name": query_object_name(),
		"id": query_id(),

		"archetype": query_archetype(),

		"environment": query_environment(),
		"inventory": query_inventory(),

		"virtual": query_virtual() ? 1 : 0,

		"mass": (v = query_mass()) ? v : nil,
		"density": query_density(),
		"flexible": query_flexible() ? 1 : 0,
		"capacity": (v = query_capacity()) ? v : nil,
		"max_mass": (v = query_max_mass()) ? v : nil,

		"properties": query_local_properties()
	]);
}

static void load(mapping data)
{
	set_object_name(data["name"]);
	set_id(data["id"]);

	set_archetype(data["archetype"]);

	if (data["virtual"]) {
		set_virtual(1);
	} else {
		mixed v;

		set_virtual(0);
		set_mass((v = data["mass"]) ? v : 0.0);
		set_density((v = data["density"]) ? v : 1.0);
		set_flexible(data["flexible"] ? 1 : 0);
		set_capacity((v = data["capacity"]) ? v : 0.0);
		set_max_mass((v = data["max_mass"]) ? v : 0.0);
	}

	set_local_properties(data["properties"]);
}

void self_destruct()
{
	destruct_object(this_object());
}
