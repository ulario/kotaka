/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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

void main(object actor, mapping roles)
{
	object env;
	object *inv;

	int i, sz;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
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

		if (roles["raw"] == exit->query_direction()) {
			object target;

			target = exit->query_destination();

			if (target) {
				"~Game/sys/action/exit"->action(
					([
						"actor": actor,
						"dob": exit
					])
				);
				return;
			} else {
				send_out("Oops, " + TEXT_SUBD->generate_brief_definite(exit)
					+ " doesn't seem to have a destination.\n"
					+ "Yell at a wizard.\n");
			}
		}
	}

	send_out("No such exit.\n");
}
