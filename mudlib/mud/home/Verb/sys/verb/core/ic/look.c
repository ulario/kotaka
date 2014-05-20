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
#include <kotaka/paths/string.h>
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
		({ "dob", ({ nil, "at" }) })
	});
}

void main(object actor, mapping roles)
{
	mixed dob;
	mixed obj;
	string prep;
	string look;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	dob = roles["dob"];

	if (!dob) {
		emit_from(actor, ({ "look", "looks" }), "around");
		send_out(RENDERD->draw_look(actor));
		return;
	}

	prep = dob[0];
	obj = dob[1];

	switch(prep) {
	case nil:
	case "at":
		if (typeof(obj) == T_OBJECT) {
			emit_from(actor, ({ "look", "looks" }), "at", obj);
			look = obj->query_property("look");
			send_out(look ? STRINGD->wordwrap(look, 60) : "a bland object");
			send_out("\n");
		} else {
			int sz, i;
			string *briefs;

			emit_from(actor, ({ "look", "looks" }), "at a bunch of stuff");

			sz = sizeof(obj);
			briefs = ({ });

			for (i = 0; i < sz; i++) {
				briefs += ({ TEXT_SUBD->generate_brief_indefinite(obj[i]) });
			}

			briefs[sz - 1] = "and " + briefs[sz - 1];

			send_out(implode(briefs, ", ") + "\n");
		}

		break;

	case "in":
		if (typeof(obj) == T_OBJECT) {
		} else {
			send_out("You have to be more specific to look inside something.\n");
		}

		emit_from(actor, ({ "look", "looks" }), "in", obj);
		{
			object *inv;
			int sz;

			inv = obj->query_inventory();
			sz = sizeof(inv);

			if (sz) {
				int i;

				send_out("Contents:\n\n");

				for (i = 0; i < sz; i++) {
					send_out(TEXT_SUBD->generate_brief_indefinite(inv[i]) + "\n");
				}

				send_out("\n");
			} else {
				send_out("There is nothing inside.\n");
			}
		}
		break;
	default:
		send_out("todo: handle preposition " + prep + "\n");
	}
}
