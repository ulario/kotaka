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
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>

inherit "/lib/string/char";
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

private void list_directory(string dir, int level)
{
	string *directories;
	string *names;
	mapping keys;
	int sz, i;

	directories = IDD->query_directories(dir);
	names = IDD->query_names(dir);

	keys = ([ ]);

	for (sz = sizeof(names); --sz >= 0; ) {
		keys[names[sz]] = 1;
	}

	for (sz = sizeof(directories); --sz >= 0; ) {
		keys[directories[sz]] = 2;
	}

	names = map_indices(keys);
	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string name;

		name = names[i];

		if (keys[name] == 2) {
			send_out(spaces(level * 4) + name + ":\n");

			if (dir) {
				list_directory(dir + ":" + name, level + 1);
			} else {
				list_directory(name, level + 1);
			}
		} else {
			send_out(spaces(level * 4) + name + "\n");
		}
	}
}

void main(object actor, mapping roles)
{
	string *users;
	object user;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to list objects.\n");

		return;
	}

	if (roles["raw"] != "") {
		list_directory(roles["raw"], 0);
	} else {
		list_directory(nil, 0);
	}
}
