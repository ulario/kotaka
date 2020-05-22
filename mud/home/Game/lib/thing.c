/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020  Raymond Jennings
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

inherit thing LIB_THING;
inherit "~Geometry/lib/thing";
inherit "thing/exit";
inherit "thing/character";
inherit "thing/living";

static void create()
{
	thing::create();
}

static mapping save()
{
	mapping map;
	object lwo;
	mixed v;

	map = ([
		"name": query_object_name(),
		"id": query_id(),

		"archetype": query_archetype(),

		"environment": query_environment(),
		"inventory": query_inventory(),

		"virtual": query_virtual() ? 1 : 0

		"mass": (v = query_mass()) ? v : nil,
		"density": query_density()),
		"flexible": query_flexible() ? 1 : 0,
		"capacity": (v = query_capacity()) ? v : nil,
		"max_mass": (v = query_max_mass()) ? v : nil,

		"properties": query_local_properties(),
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

static void load(mapping data)
{
	mixed arch;
	mixed map;
	object lwo;

	set_object_name(data["name"]);
	set_id(data["id"]);

	if (arch = data["archetype"]) {
		set_archetype(arch);
	} else if (arch = data["archetypes"]) {
		if (sizeof(arch)) {
			set_archetype(arch[0]);
		} else {
			set_archetype(nil);
		}
	}

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

	if (map = data["character"]) {
		initialize_character(map["attack"], map["defense"], map["maxhp"]);
	}

	if (map = data["living"]) {
		initialize_living();
		lwo = query_living_lwo();
		lwo->set_hp(map["hp"]);
	}
}
