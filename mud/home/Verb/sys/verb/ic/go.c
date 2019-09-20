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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_EMIT;
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Go";
}

string *query_help_contents()
{
	return ({
		"Goes through an exit.",
		"Specify the exit by direction or building or whatever.",
		"Many exits are named by compass directions, but not all of them.",
		"In cases of buildings, the exit may be named after the building."
	});
}

void main(object actor, mapping roles)
{
	object env;
	object *inv;

	int i, sz;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	if (!actor->query_character_lwo()) {
		send_out("You are not a character.\n");
		return;
	}

	if (!actor->query_living_lwo()) {
		send_out("You're dead, you can't do that.\n");
		return;
	}

	env = actor->query_environment();

	if (!env) {
		send_out("Oh dear, you are outside reality itself and there's nowhere to go.\n");
		return;
	}

	inv = env->query_inventory();
	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		object exit;

		exit = inv[i];

		if (roles["raw"] == exit->query_property("exit_direction")) {
			"~Action/sys/action/exit"->action(
				([
					"actor": actor,
					"dob": exit
				])
			);
			return;
		}
	}

	send_out("No such exit.\n");
}
