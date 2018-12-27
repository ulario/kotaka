/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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

string query_help_title()
{
}

string *query_help_contents()
{
}

void main(object actor, mapping roles)
{
	mixed dob;
	string look;
	object *worn;
	string *parts;
	int i, sz, dlayer;

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

	if (dob == actor) {
		send_out("That doesn't make any sense.\n");
		return;
	}

	if (dob->query_environment() != actor) {
		send_out("You don't have it.\n");
		return;
	}

	if (!dob->query_property("is_worn")) {
		send_out("You are not wearing it.\n");
		return;
	}

	if (dob->query_property("is_immobile")) {
		send_out("It is stuck like glue and cannot be worn.\n");
		return;
	}

	worn = actor->query_inventory() - ({ dob });
	parts = dob->query_property("parts_covered");

	sz = sizeof(worn);

	for (i = 0; i < sz; i++) {
		int layer;

		if (!worn[i]->query_property("is_worn")) {
			continue;
		}

		if (!sizeof(worn[i]->query_property("parts_covered") & parts)) {
			continue;
		}

		layer = worn[i]->query_property("clothing_layer");

		if (layer > dlayer) {
			send_out("You have to remove your " + worn[i]->query_property("brief") + " first.\n");
			return;
		}
	}

	emit_from(actor, actor, " ", ({ "remove", "removes" }), " ", dob, ".");
	dob->set_property("is_worn", nil);
	dob->set_property("clothing_layer", nil);
}
