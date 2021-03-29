/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2021  Raymond Jennings
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
#include <game/paths.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_EMIT;
inherit LIB_VERB;
inherit "~/lib/ic";

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Craft";
}

string *query_help_contents()
{
	return ({
		"Crafts an object.",
		"Craft by itself lists craftable items."
	});
}

atomic void main(object actor, mapping roles)
{
	string args;
	object master;
	object item;

	args = roles["raw"];

	if (!args) {
		send_out("Usage: craft <item>\n");
		return;
	}

	switch(args) {
	case "list":
		send_out("You can craft:\n");
		send_out("club - A simple wooden club\n");
		break;

	case "club":
		master = IDD->find_object_by_name("class:weapon:club");

		if (!master) {
			send_out("Yell at a wizard, the master club blueprint is missing.\n");
			break;
		}

		emit_from(actor, actor, ({ " craft", " crafts" }), " a wooden club.");

		item = GAME_INITD->create_thing();
		item->set_archetype(master);
		item->set_id_base("club");
		item->move(actor);
		break;
	}
}
