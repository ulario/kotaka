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
	return ({
		({ "dob", ({ nil }), 0 }),
		({ "iob", ({ "in" }), 0 })
	});
}

private string validate_put(object actor, object obj, object iob)
{
	if (obj == actor) {
		return "Your attempt at contortion is unacceptable.";
	}

	if (obj->query_environment() != actor) {
		return "It has to be in your possession first.";
	}

	if (obj->is_container_of(iob)) {
		return "Quantum implosion prevented.";
	}

	if (obj == iob) {
		return "Just turn it inside out.";
	}

	if (obj->query_property("is_immobile")) {
		return "It is stuck like glue and cannot be put anywhere.";
	}
}

private void do_put(object actor, object obj, object iob)
{
	obj->move(iob);
	obj->set_x_position(0);
	obj->set_y_position(0);
	obj->set_z_position(0);
}

void main(object actor, mapping roles)
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

	if (!dob) {
		send_out("put what?\n");
		return;
	}

	if (!iob) {
		send_out("Put it where?\n");
		return;
	}

	if (dob[0]) {
		send_out("Bad grammar.\n");
		return;
	}

	if (iob[0] != "in") {
		send_out("Bad grammar.\n");
		return;
	}

	dob = dob[1];
	iob = iob[1];

	switch(typeof(dob)) {
	case T_STRING:
		send_out(dob + "\n");
		return;

	case T_ARRAY:
	case T_OBJECT:
		switch(typeof(iob)) {
		case T_STRING:
			send_out(iob + "\n");
			return;

		case T_ARRAY:
			send_out("Please be more specific on who or what you are putting things in.\n");
			return;

		case T_OBJECT:
			switch(typeof(dob)) {
			case T_ARRAY:
				{
					int i, sz;
					sz = sizeof(dob);

					for (i = 0; i < sz; i++) {
						string err;

						err = validate_put(actor, dob[i], iob);

						if (err) {
							send_out(err + "\n");
							return;
						}
					}

					for (i = 0; i < sz; i++) {
						do_put(actor, dob[i], iob);
					}
				}
				break;

			case T_OBJECT:
				{
					string err;

					err = validate_put(actor, dob, iob);

					if (err) {
						send_out(err + "\n");
						return;
					}

					do_put(actor, dob, iob);
				}
				break;
			}
		}
	}

	emit_from(actor, actor, " ", ({ "put", "puts" }), dob, " in ", iob, ".");
}
