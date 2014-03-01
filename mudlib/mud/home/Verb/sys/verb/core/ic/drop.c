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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <type.h>

inherit LIB_EMIT;
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "english" });
}

/* ({ role, prepositions, raw }) */
mixed **query_roles()
{
	return ({ });
}

void main(object actor, mapping roles)
{
	mixed dob;
	string look;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	dob = roles["dob"];

	if (!dob) {
		send_out("Take what?\n");
		return;
	}

	if (typeof(dob) == T_STRING) {
		send_out(dob + "\n");
		return;
	}

	if (dob[0]) {
		send_out("Your grammar stinks.\n");
		return;
	}

	dob = dob[1];

	if (dob == actor) {
		send_out("Try as you might, you cannot lift yourself.\n");
		return;
	}

	if (!actor->query_environment()) {
		send_out("It isn't polite to litter in the void.\n");
		return;
	}

	if (dob->query_environment() == actor->query_environment()) {
		send_out("It is already on the ground!\n");
		return;
	}

	if (dob->query_property("is_immobile")) {
		send_out("It is stuck like glue and cannot be dropped.\n");
		return;
	}

	if (dob->query_environment() == actor) {
		dob->move(actor->query_environment());
		dob->set_x_position(actor->query_x_position());
		dob->set_y_position(actor->query_y_position());
		dob->set_z_position(actor->query_z_position());
		generic_emit(actor, ({ "drop", "drops" }), dob, nil);
		return;
	}

	send_out("It is out of reach.\n");
}
