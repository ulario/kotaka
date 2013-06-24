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
#include <kotaka/paths.h>
#include <text/paths.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
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

		if (args == exit->query_direction()) {
			object target;

			target = exit->query_destination();

			if (target) {
				/* todo: walk to target */
				actor->set_x_position(exit->query_x_position());
				actor->set_y_position(exit->query_y_position());
				actor->set_z_position(exit->query_z_position());
				actor->move(target);
				generic_emit(actor, ({ "go", "goes" }), exit, "through");
				return;
			}
		}
	}

	send_out("No such exit.\n");
}
