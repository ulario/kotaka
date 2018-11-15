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
#include <kotaka/paths/intermud.h>
#include <kotaka/paths/verb.h>

inherit "/lib/string/format";
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	string name;
	string *list;

	object telnet;
	int width;

	object user;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You have insufficient access to list i3 muds.\n");
		return;
	}

	if (!find_object(INTERMUDD)) {
		send_out("IntermudD is offline.\n");
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

			table[0] = ({ "Name", "IP", "Port", "Status" });

			for (i = 0; i < sz; i++) {
				mixed *info;

				info = INTERMUDD->query_mud(list[i]);

				if (info[0] == -1) {
					table[i + 1] = ({ list[i], info[1], info[2] + "", info[0] + "" });
				}
			}

			table -= ({ nil });

			send_out(render_table(table, 2) + "\n");
		}
		break;

	case nil:
	case "":
		send_out(wordwrap(implode(list, ", "), width) + "\n");
		break;

	default:
		send_out("Usage: imudlist [-v]\n");
	}
}
