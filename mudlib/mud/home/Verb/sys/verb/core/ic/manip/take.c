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

string validate_take(object actor, object obj)
{
	if (obj->is_container_of(actor)) {
		return "You can't take something you're inside of.";
	}

	if (obj->query_environment() == actor) {
		return "You already have it.";
	}

	if (obj == actor) {
		return "You can't take yourself.";
	}

	if (obj->query_property("is_immobile")) {
		return "It is stuck like glue and cannot be taken.\n";
	}
}

void main(object actor, mapping roles)
{
	mixed dob;
	string look;
	object from;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	dob = roles["dob"];

	if (!dob) {
		send_out("Take what?\n");
		return;
	}

	if (dob[0]) {
		send_out("Your grammar stinks.\n");
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

			err = validate_take(actor, dob);

			if (err) {
				send_out(err + "\n");
				return;
			}

			if (dob->query_environment() != actor->query_environment()) {
				from = dob->query_environment();
			}

			dob->move(actor);
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

				err = validate_take(actor, obj);

				if (err) {
					send_out(err + "\n");
					return;
				}
			}

			if (dob[0]->query_environment() != actor->query_environment()) {
				from = dob[0]->query_environment();
			}

			/* execute */
			for (i = 0; i < sz; i++) {
				dob[i]->move(actor);
			}
		}
		break;
	}

	if (from) {
		emit_from(actor, actor, " ", ({ "take", "takes" }), dob, " from ", from, ".");
	} else {
		emit_from(actor, actor, " ", ({ "take", "takes" }), " ", dob, ".");
	}
}
