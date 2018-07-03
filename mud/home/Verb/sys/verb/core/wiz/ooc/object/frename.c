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

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

atomic private void do_folder_rename(string old_name, string new_name)
{
	mapping map;
	string *names;
	int *keys;
	int sz, i;

	map = CATALOGD->list_directory(old_name);

	names = map_indices(map);
	keys = map_values(map);

	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string name;

		name = names[i];

		switch(keys[i]) {
		case 1: /* object */
			CATALOGD->lookup_object(old_name + ":" + name)->set_object_name(new_name + ":" + name);
			break;
		case 2:
			do_folder_rename(old_name + ":" + name, new_name + ":" + name);
		}
	}
}

void main(object actor, mapping roles)
{
	string old_name, new_name;
	object user;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to rename catalog folders.\n");
		return;
	}

	if (sscanf(roles["raw"], "%s %s", old_name, new_name) != 2) {
		send_out("Usage: frename old_folder new_folder\n");
		return;
	}

	switch (CATALOGD->test_name(old_name)) {
	case -2: send_out(old_name + " has an object in it.\n"); return;
	case -1: send_out(old_name + " does not exist somewhere.\n"); return;
	case 0: send_out(old_name + " does not exist.\n"); return;
	case 1: send_out(old_name + " is an object.\n"); return;
	}

	switch (CATALOGD->test_name(new_name)) {
	case -2: send_out(new_name + " has an object in it.\n"); return;
	case 2: send_out(new_name + " already exists.\n"); return;
	case 1: send_out(new_name + " is an object.\n"); return;
	}

	do_folder_rename(old_name, new_name);
}
