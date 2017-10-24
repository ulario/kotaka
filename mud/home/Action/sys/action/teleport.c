/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kotaka/paths/action.h>

inherit LIB_USERIO;
inherit LIB_EMIT;
inherit LIB_ACTION;

private void fix_position(object actor)
{
	object origin;
	string csystem;
	object env;

	int px;
	int py;
	int pz;

	origin = actor->query_outer_origin();

	if (!origin) {
		actor->clear_xyz();
		return;
	}

	csystem = origin->query_coordinate_system();

	if (csystem != "xyz") {
		actor->clear_xyz();
		return;
	}

	env = actor->query_environment();

	if (!env) {
		actor->clear_xyz();
		return;
	}

	actor->set_x_position((env->query_x_size() - 1) / 2);
	actor->set_y_position((env->query_y_size() - 1) / 2);
	actor->set_z_position(0);
}

void action(mapping roles)
{
	object exit;
	object target;
	object actor;

	object origin;

	actor = roles["actor"];
	target = roles["dob"];

	if (actor->query_property("is_immobile")) {
		send_out("You're stuck like glue and can't move.\n");
		return;
	}

	emit_from(actor, actor, " ", ({ "vanish", "vanishes" }), " into thin air.");
	actor->move(target);
	fix_position(actor);

	emit_from(actor, actor, " ", ({ "appear", "appears" }), " out of thin air.");
}
