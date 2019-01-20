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
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Ansidemo";
}

string *query_help_contents()
{
	return ({ "Shows off the pretty ansi video subsystem.", "Can show the matrix effect, sparks, and flying through space." });
}

void main(object actor, mapping roles)
{
	string demo;

	demo = roles["raw"];

	if (demo == "") {
		send_out("Usage: ansidemo <demo>\n");
		send_out("Available demos:\n");
		send_out("matrix - Demonstrate a cascading field of ones and zeroes in eerie green hues\n");
		send_out("spark  - Create a volcano of fiery sparks\n");
		send_out("space  - Fly through a field of stars that slowly spins as you proceed\n");
		return;
	}

	if (find_object("~Text/obj/ustate/movie/" + demo)) {
		query_ustate()->push_state(query_user()->clone_ustate("movie/" + demo));
	} else {
		send_out("No such ansi demo\n");
	}
}
