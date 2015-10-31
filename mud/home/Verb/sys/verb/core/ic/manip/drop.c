/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014, 2015  Raymond Jennings
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

private string validate_drop(object actor, object obj)
{
	if (obj->query_environment() != actor) {
		return "You can only drop something if you have it.";
	}
}

private void do_drop(object actor, object obj)
{
	obj->move(actor->query_environment());
	obj->set_x_position(actor->query_x_position());
	obj->set_y_position(actor->query_y_position());
	obj->set_z_position(actor->query_z_position());
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
		send_out("Drop what?\n");
		return;
	}

	if (typeof(dob) == T_STRING) {
		send_out(dob + "\n");
		return;
	}

	if (dob[0]) {
		send_out("You cannot use a preposition with a direct object in this verb.\n");
		return;
	}

	dob = dob[1];

	switch(typeof(dob)) {
	case T_STRING:
		send_out(dob + "\n");
		return;

	case T_OBJECT:
		{
			string err;

			err = validate_drop(actor, dob);

			if (err) {
				send_out(err + "\n");
				return;
			}

			do_drop(actor, dob);
		}
		break;

	case T_ARRAY:
		{
			object obj;
			int sz, i;

			sz = sizeof(dob);

			/* validate */
			for (i = 0; i < sz; i++) {
				string err;

				obj = dob[i];

				err = validate_drop(actor, obj);

				if (err) {
					send_out(err + "\n");
					return;
				}
			}

			/* execute */
			for (i = 0; i < sz; i++) {
				do_drop(actor, dob[i]);
			}
		}
		break;
	}

	emit_from(actor, actor, " ", ({ "drop", "drops" }), " ", dob, ".");
}
