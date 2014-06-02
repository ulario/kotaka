/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <game/paths.h>

inherit LIB_EMIT;
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

atomic void main(object actor, mapping roles)
{
	object master;
	object thing;
	int num;
	string path;

	if (query_user()->query_class() < 2) {
		send_out("You must be a wizard to use this command.\n");
		return;
	}

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	if (sscanf(roles["raw"], "%d %s", num, path) != 2) {
		send_out("Usage: multispawn <quantity> <path>\n");
	}

	master = CATALOGD->lookup_object(path);

	if (!master) {
		send_out("No such object: " + path + ".\n");
		return;
	}

	while (num) {
		thing = GAME_INITD->create_object();
		thing->add_archetype(master);
		thing->move(actor);
		num--;
	}

	emit_from(actor, actor, " ", ({ "spawn", "spawns" }), " ", thing, " ", num + "times.");
}
