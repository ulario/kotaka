/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/paths.h>
#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

private void list_directory(string dir, int level)
{
	mapping list;
	int *keys;
	string *names;
	int i, sz;

	list = CATALOGD->list_directory(dir);

	names = map_indices(list);
	keys = map_values(list);

	sz = sizeof(keys);

	for (i = 0; i < sz; i++) {
		if (keys[i] == 2) {
			send_out(STRINGD->spaces(level * 4) + names[i] + " {\n");

			if (dir) {
				list_directory(dir + ":" + names[i], level + 1);
			} else {
				list_directory(names[i], level + 1);
			}

			send_out(STRINGD->spaces(level * 4) + "}\n");
		} else {
			send_out(STRINGD->spaces(level * 4) + names[i] + "\n");
		}
	}
}

void main(object actor, string args)
{
	string *users;
	object user;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to list accounts.\n");
		return;
	}

	list_directory(nil, 0);
}
