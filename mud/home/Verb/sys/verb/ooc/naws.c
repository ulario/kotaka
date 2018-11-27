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
	string args;

	args = roles["raw"];
	conn = query_user();
	conn = conn->query_telnet_obj();

	switch(args) {
	case "on":
		if (conn) {
			send_out("Enabling NAWS.\n");
			conn->send_do(31);
		} else {
			send_out("Error: no telnet filter detected on this connection\n");
		}
		break;

	case "off":
		if (conn) {
			send_out("Disabling NAWS.\n");
			conn->send_dont(31);
		} else {
			send_out("Error: no telnet filter detected on this connection\n");
		}
		break;

	default:
		send_out("Usage: naws <on|off>\n");
		return;
	}
}
