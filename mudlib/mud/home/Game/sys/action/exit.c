/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <game/paths.h>

inherit LIB_USERIO;
inherit LIB_EMIT;
inherit LIB_ACTION;

void action(mapping roles)
{
	object exit;
	object target;
	object actor;

	actor = roles["actor"];
	exit = roles["dob"];
	target = exit->query_property("exit_destination");

	if (!target) {
		send_out("Oops, " + TEXT_SUBD->query_brief_definite(exit) + " doesn't have a destination.\n");
		return;
	}

	if (actor->query_property("is_immobile")) {
		send_out("You're stuck like glue and can't move.\n");
		return;
	}

	emit_from(actor, ({ "leave", "leaves" }), "through", exit);
	actor->set_x_position(exit->query_x_position());
	actor->set_y_position(exit->query_y_position());
	actor->set_z_position(exit->query_z_position());
	actor->move(target);
	emit_from(actor, ({ "arrive", "arrives" }));
}
