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
#include <kotaka/paths/intermud.h>
#include <kotaka/paths/verb.h>

inherit "/lib/string/format";
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "imudlist";
}

string *query_help_contents()
{
	return ({ "Usage: imudlist [-v]", "Shows a list of all muds on i3.", "-v: Verbose listing, show IP and port information in a table." });
}

void main(object actor, mapping roles)
{
	int width;
	object telnet, user;
	string *list, name;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You have insufficient access to list i3 muds.\n");
		return;
	}

	if (roles["raw"]) {
		send_out("Usage: imudlist\n");
		return;
	}

	list = INTERMUDD->query_muds();

	if (!list) {
		send_out("IntermudD is down.\n");
		return;
	}

	telnet = user->query_telnet_obj();

	width = 80;

	if (telnet) {
		if (telnet->query_naws_active()) {
			width = telnet->query_naws_width();
		}
	}

	switch(roles["raw"]) {
	case "-v":
		{
			string **table;
			int sz, i;

			sz = sizeof(list);

			table = allocate(sz + 1);

			table[0] = ({ "Name", "IP", "Port" });

			for (i = 0; i < sz; i++) {
				mixed *info;

				info = INTERMUDD->query_mud(list[i]);

				if (info[0] == -1) {
					table[i + 1] = ({ list[i], info[1], info[2] + "" });
				}
			}

			table -= ({ nil });

			send_out(render_table(table, 2) + "\n");
		}
		break;

	case nil:
		send_out(wordwrap(implode(list, ", "), width) + "\n");
		break;

	default:
		send_out("Usage: imudlist [-v]\n");
	}
}
