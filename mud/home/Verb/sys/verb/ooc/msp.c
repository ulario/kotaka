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
	return "Msp";
}

string *query_help_contents()
{
	return ({ "Manipulates the Mud Sound Protocol." });
}

void main(object actor, mapping roles)
{
	object obj;
	string args;

	args = roles["raw"];
	obj = query_user();
	obj = obj->query_mudclient_obj();

	switch(args) {
	case "on":
		if (obj) {
			send_out("Enabling MSP.\n");
			obj->enable_msp();
		} else {
			send_out("No mudclient filter detected on this connection\n");
		}
		break;

	case "off":
		if (obj) {
			send_out("Disabling MSP.\n");
			obj->disable_msp();
		} else {
			send_out("No mudclient filter detected on this connection\n");
		}
		break;

	case "test":
		if (obj) {
			send_out("Testing MSP.\n");
			obj->beep();
		} else {
			send_out("No mudclient filter detected on this connection\n");
		}
		break;

	default:
		send_out("Usage: msp <on|off|test>\n");
	}
}
