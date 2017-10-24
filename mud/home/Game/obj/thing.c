/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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

inherit "../lib/thing";

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

mapping save()
{
	ACCESS_CHECK(GAME());

	return ([
		"archetypes": query_archetypes(),
		"capacity": query_capacity(),
		"density": query_density(),
		"environment": query_environment(),
		"flexible": query_flexible(),
		"id": query_id(),
		"inventory": query_inventory(),
		"mass": query_mass(),
		"max_mass": query_max_mass(),
		"name": query_object_name(),
		"properties": query_local_properties(),
		"virtual": query_virtual()
	]);
}

void load(mapping data)
{
	ACCESS_CHECK(GAME());

	set_archetypes(data["archetypes"]);
	set_virtual(data["virtual"]);
	set_capacity(data["capacity"]);
	set_density(data["density"]);
	set_flexible(data["flexible"]);
	set_id(data["id"]);
	rearrange_inventory(data["inventory"]);
	set_mass(data["mass"]);
	set_max_mass(data["max_mass"]);
	set_local_properties(data["properties"]);
}
