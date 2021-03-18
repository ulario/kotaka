/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <kotaka/paths/system.h>

inherit "/lib/string/case";
inherit LIB_EMIT;
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Live";
}

string *query_help_contents()
{
	return ({ "Resurrect after dying.  Requires your body to still exist, otherwise you'll have to reincarnate." });
}

void main(object actor, mapping roles)
{
	object user;
	string name;

	object template;
	object ghost;
	object body;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	user = query_user();
	name = user->query_name();

	template = IDD->find_object_by_name("templates:" + name);

	if (!template) {
		send_out("You don't have a character, please run chargen.\n");
		return;
	}

	ghost = IDD->find_object_by_name("ghosts:" + name);

	if (!ghost) {
		send_out("Sadness...your soul has been consigned to oblivion.\n");
		return;
	}

	body = IDD->find_object_by_name("players:" + name);

	if (!body) {
		send_out("Bummer, your body is gone.  Looks like you're stuck in the afterlife.\n");
		return;
	}

	if (!body->query_character_lwo()) {
		send_out("BUG: Your body doesn't appear to be a valid character, yell at a wizard.\n");
		return;
	}

	if (body->query_living_lwo()) {
		object possessee;

		send_out("You're already alive.\n");

		possessee = ghost->query_possessee();

		if (!possessee) {
			send_out("But your soul wasn't attached properly, fixing that...\n");

			ghost->move(body);
			ghost->possess(body);
		} else if (possessee != body) {
			send_out("Somehow you're possessing something other than your natural body...\n");
		}
	} else {
		object possessee;

		possessee = ghost->query_possessee();

		if (possessee) {
			if (possessee != body) {
				send_out("Somehow you're possessing something other than your natural body...\n");
				return;
			} else {
				send_out("Weird, you were already possessing your body.\n");
			}
		} else {
			send_out("Returning your soul to its body...\n");

			emit_from(ghost, ghost, " ", ({ "enter", "enters" }), " ", body);
			ghost->move(body);
			ghost->possess(body);

			send_out("Reviving you...\n");
			body->initialize_living();
			body->set_local_description(nil, "brief", to_title(name));
			emit_from(body, body, " ", ({ "revive", "revives" }), "!");
		}
	}
}
