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
#include <kernel/user.h>
#include <kotaka/paths/verb.h>
#include <kotaka/telnet.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Naws";
}

string *query_help_contents()
{
	return ({ "Manipulates the Negotiate About Window Size telnet option." });
}

void main(object actor, mapping roles)
{
	object obj;
	string args;

	args = roles["raw"];
	obj = query_user();
	obj = obj->query_telnet_obj();

	switch(args) {
	case "on":
		if (obj) {
			send_out("Enabling NAWS\n");
			obj->send_do(31);
		} else {
			send_out("No telnet filter detected on this connection\n");
		}
		break;

	case "off":
		if (obj) {
			send_out("Disabling NAWS\n");
			obj->send_dont(31);
		} else {
			send_out("No telnet filter detected on this connection\n");
		}
		break;

	case "status":
		if (obj) {
			send_out("NAWS pending: " + (obj->query_naws_pending() ? "yes" : "no") + "\n");
			send_out("NAWS active: " + (obj->query_naws_active() ? "yes" : "no") + "\n");
			send_out("NAWS width: " + obj->query_naws_width() + "\n");
			send_out("NAWS height: " + obj->query_naws_height() + "\n");
			send_out("NAWS telnet debug flags: " + obj->query_telnet_debug() + "\n");
		} else {
			send_out("No telnet filter detected on this connection\n");
		}
		break;

	default:
		send_out("Usage: naws <on|off|status>\n");
	}
}
