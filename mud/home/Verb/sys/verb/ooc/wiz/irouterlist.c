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
	object user;
	string *list;
	string **table;
	int sz, i;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You have insufficient access to list I3 routers.\n");
		return;
	}

	if (!find_object(INTERMUDD)) {
		send_out("IntermudD is offline.\n");
		return;
	}

	list = INTERMUDD->query_routers();

	if (!list) {
		send_out("IntermudD is down.\n");
		return;
	}

	sz = sizeof(list);

	table = allocate(sz + 1);

	table[0] = ({ "Name", "IP", "Port" });

	for (i = 0; i < sz; i++) {
		mixed *info;

		info = INTERMUDD->query_router(list[i]);

		table[i + 1] = ({ list[i], info[0], info[1] + "" });
	}

	table -= ({ nil });

	send_out(render_table(table, 2) + "\n");
}
