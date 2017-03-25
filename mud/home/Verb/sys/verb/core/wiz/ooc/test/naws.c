/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kernel/user.h>
#include <kotaka/paths/verb.h>
#include <kotaka/telnet.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	object conn;
	string out;

	send_out("Testing NAWS option...\n");

	conn = query_user();

	out = "   ";

	out[0] = TELNET_IAC;
	out[1] = TELNET_DO;
	out[2] = 31;

	while (conn && conn <- LIB_USER) {
		if (conn <- "~Text/obj/filter/telnet") {
			conn->send_raw(out);
			return;
		}
		conn = conn->query_conn();
	}

	send_out(out);
}
