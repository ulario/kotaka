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
#include <type.h>

inherit LIB_EMIT;
inherit LIB_ENGVERB;

/* ({ role, prepositions, raw }) */
mixed **query_roles()
{
	return ({
		({ "iob", ({ "in" }), 0 })
	});
}

void do_action(object actor, mapping roles, string evoke)
{
	mixed dob;
	mixed iob;
	string look;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	dob = roles["dob"];
	iob = roles["iob"];

	if (dob == actor) {
		send_out("Try as you might, you cannot lift yourself.\n");
		return;
	}

	if (!dob) {
		send_out("Put what?\n");
		return;
	}

	if (typeof(dob) == T_STRING) {
		send_out(dob + "\n");
		return;
	}

	if (!iob) {
		send_out("Put it where?\n");
		return;
	}

	if (typeof(iob) == T_STRING) {
		send_out(iob + "\n");
		return;
	}

	if (dob[0]) {
		send_out("Your grammar stinks.\n");
		return;
	}

	dob = dob[1];

	if (dob->query_environment() != actor) {
		send_out("It doesn't appear to be in your hands.\n");
		return;
	}

	iob = iob[1];

	if (dob->query_environment() == actor) {
		if (dob->is_container_of(iob)) {
			send_out("Turn it inside out first.\n");
			return;
		}

		if (dob == iob) {
			send_out("Just turn it inside out.\n");
			return;
		}

		if (dob->query_property("is_immobile")) {
			send_out("It is stuck like glue and cannot be put anywhere.\n");
			return;
		}

		emit_from(actor, ({ "put", "puts" }), dob, "in", iob);
		dob->move(iob);
		dob->set_x_position(0);
		dob->set_y_position(0);
		dob->set_z_position(0);
		return;
	}
}
