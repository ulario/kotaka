/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014, 2015, 2017  Raymond Jennings
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

void main(object actor, mapping roles)
{
	string demo;

	if (roles["raw"] == "") {
		send_out("Usage: ansidemo <demo>\n");
		send_out("Available demos:\n");
		send_out("matrix - Demonstrate a cascading field of ones and zeroes in eerie green hues\n");
		send_out("spark  - Create a volcano of fiery sparks\n");
		send_out("space  - Fly through a field of stars that slowly spins as you proceed\n");
		return;
	}

	switch(roles["raw"]) {
	case "matrix":
	case "sparks":
	case "space":
		demo = roles["raw"];
		break;
	default:
		send_out("No such ansi demo\n");
		return;
	}

	query_ustate()->push_state(query_user()->create_ustate("movie/" + demo));
}
