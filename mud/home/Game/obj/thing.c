/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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

static void destruct(int clone)
{
	if (clone) {
		::destruct();
	}
}

mapping save()
{
	mapping map;
	object lwo;

	ACCESS_CHECK(GAME());

	map = ([
		"archetype": query_archetype(),
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

	if (lwo = query_character_lwo()) {
		map["character"] = ([
			"attack": lwo->query_attack(),
			"defense": lwo->query_defense(),
			"maxhp": lwo->query_max_hp()
		]);
	}

	if (lwo = query_living_lwo()) {
		map["living"] = ([
			"hp": lwo->query_hp()
		]);
	}

	return map;
}

void load(mapping data)
{
	mixed arch;
	mixed map;
	object lwo;

	ACCESS_CHECK(GAME());

	if (arch = data["archetype"]) {
		set_archetype(arch);
	} else if (arch = data["archetypes"]) {
		if (sizeof(arch)) {
			set_archetype(arch[0]);
		} else {
			set_archetype(nil);
		}
	}

	set_virtual(data["virtual"]);
	set_capacity(data["capacity"]);
	set_density(data["density"]);
	set_flexible(data["flexible"]);
	set_id(data["id"]);
	set_mass(data["mass"]);
	set_max_mass(data["max_mass"]);
	set_object_name(data["name"]);
	set_local_properties(data["properties"]);
	set_object_name(data["name"]);

	if (map = data["character"]) {
		initialize_character(map["attack"], map["defense"], map["maxhp"]);
	}

	if (map = data["living"]) {
		initialize_living();
		lwo = query_living_lwo();
		lwo->set_hp(map["hp"]);
	}
}
