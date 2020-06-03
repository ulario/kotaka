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
inherit "thing/possess";
inherit detail "thing/detail";
inherit "thing/prox";

static void create()
{
	thing::create();
	detail::create();
}

static mapping save()
{
	mapping map;
	object lwo;
	mixed v;

	map = ::save();

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

	map["possessee"] = query_possessee();
	map["details"] = detail_save();
	map["prox"] = prox_save();

	return map;
}

static void load(mapping data)
{
	mapping map;
	object pos;

	::load(data);

	if (map = data["character"]) {
		initialize_character(map["attack"], map["defense"], map["maxhp"]);
	}

	if (map = data["living"]) {
		object lwo;

		initialize_living();
		lwo = query_living_lwo();
		lwo->set_hp(map["hp"]);
	}

	if (pos = data["possessee"]) {
		possess(pos);
	}

	prox_load(data["prox"]);
	detail_load(data["details"]);
}

void patch_details()
{
	string brief;
	string look;

	string *snouns;
	string *pnouns;
	string *adjectives;

	brief = query_local_property("brief");
	look = query_local_property("look");

	snouns = query_local_property("local_snouns");
	pnouns = query_local_property("local_pnouns");
	adjectives = query_local_property("local_adjectives");

	if (brief || look || snouns || pnouns || adjectives) {
		if (!has_detail(nil)) {
			add_detail(nil);
		}

		if (brief) {
			set_description(nil, "brief", brief);
		}

		if (look) {
			set_description(nil, "look", look);
		}

		if (snouns) {
			int sz;

			for (sz = sizeof(snouns); --sz >= 0; ) {
				add_snoun(nil, snouns[sz]);
			}
		}

		if (pnouns) {
			int sz;

			for (sz = sizeof(pnouns); --sz >= 0; ) {
				add_pnoun(nil, pnouns[sz]);
			}
		}

		if (adjectives) {
			int sz;

			for (sz = sizeof(adjectives); --sz >= 0; ) {
				add_adjective(nil, adjectives[sz]);
			}
		}
	}
}
