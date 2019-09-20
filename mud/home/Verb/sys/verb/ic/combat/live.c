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

inherit LIB_EMIT;
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

/*
mixed **query_roles()
{
	return ({
		({ "iob", ({ "to" }), 0 })
	});
}
*/

string query_help_title()
{
}

string *query_help_contents()
{
}

void main(object actor, mapping roles)
{
	object achar;
	object aliv;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	achar = actor->query_character_lwo();

	if (!achar) {
		send_out("BUG: You are not a character.  Please contact a wizard.\n");
		return;
	}

	aliv = actor->query_living_lwo();

	if (aliv) {
		send_out("You're already alive.\n");
		return;
	}

	actor->initialize_living();

	send_out("You're alive!\n");
}
