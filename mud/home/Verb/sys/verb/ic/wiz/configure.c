/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020, 2021  Raymond Jennings
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

inherit LIB_VERB;
inherit "~Text/lib/sub";

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
	return "Configure";
}

string *query_help_contents()
{
	return ({ "Starts the object configure interface on an object." });
}

void main(object actor, mapping roles)
{
	object configure;
	mixed dob;

	if (query_user()->query_class() < 2) {
		send_out("You must be a wizard to use this command.\n");
		return;
	}

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	dob = roles["dob"];

	if (!dob) {
		send_out("Configure what?\n");
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

	if (typeof(dob) == T_ARRAY) {
		send_out("Be more specific.\n");
		return;
	}

	send_out("You start to configure " + generate_brief_definite(dob) + ".\n");

	configure = query_user()->clone_ustate("wiz/objconf");
	configure->set_object(dob);
	query_ustate()->push_state(configure);
}
