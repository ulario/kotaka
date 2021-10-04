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
inherit "~/lib/ic";

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
	return "Weigh";
}

string *query_help_contents()
{
	return ({ "Gives you how much an object in your inventory weighs, including its contents." });
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
		send_out("Weigh what?\n");
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

	if (dob->query_environment() != actor) {
		send_out("It doesn't appear to be in your hands.\n");
		return;
	}

	if (dob->query_environment() == actor) {
		float mass;

		emit_from(actor, actor, " ", ({ "weigh", "weighs" }), " ", dob, ".");
		mass = dob->query_total_mass();

		if (mass < 1.0) {
			send_out("It weighs " + mass * 1000.0 + " grams.\n");
		} else if (mass < 1000.0) {
			send_out("It weighs " + mass + " kilograms.\n");
		} else if (mass < 1.0e+6) {
			send_out("It weighs " + mass / 1000.0 + " metric tons.\n");
		} else {
			send_out("It weighs " + mass / 1.0e+6 + " metric kilotons.\n");
		}
		return;
	}
}
