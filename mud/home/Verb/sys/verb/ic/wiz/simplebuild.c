/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Simplebuild";
}

string *query_help_contents()
{
	return ({ "Invokes the simplebuild interface" });
}

void main(object actor, mapping roles)
{
	object build;
	mixed dob;

	if (query_user()->query_class() < 2) {
		send_out("You must be a wizard to use this command.\n");
		return;
	}

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	send_out("You start to simplebuild.\n");

	build = query_user()->clone_ustate("wiz/simplebuild");
	build->set_body(actor);
	query_ustate()->push_state(build);
}
