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
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

atomic private void do_folder_rename(string old, string new)
{
	mapping map;
	string *names;
	int *keys;
	int sz, i;

	map = CATALOGD->list_directory(old);

	names = map_indices(map);
	keys = map_values(map);

	sz = sizeof(names);

	for (i = 0; i < sz; i++) {
		string name;

		name = names[i];

		switch(keys[i]) {
		case 1: /* object */
			CATALOGD->lookup_object(old + ":" + name)->set_object_name(new + ":" + name);
			break;
		case 2:
			do_folder_rename(old + ":" + name, new + ":" + name);
		}
	}
}

void main(object actor, mapping roles)
{
	string old, new;
	object user;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to rename catalog folders.\n");
		return;
	}

	if (sscanf(roles["raw"], "%s %s", old, new) != 2) {
		send_out("Usage: frename old_folder new_folder\n");
		return;
	}

	switch (CATALOGD->test_name(old)) {
	case -2: send_out(old + " has an object in it.\n"); return;
	case -1: send_out(old + " does not exist somewhere.\n"); return;
	case 0: send_out(old + " does not exist.\n"); return;
	case 1: send_out(old + " is an object.\n"); return;
	}

	switch (CATALOGD->test_name(new)) {
	case -2: send_out(new + " has an object in it.\n"); return;
	case 2: send_out(new + " already exists.\n"); return;
	case 1: send_out(new + " is an object.\n"); return;
	}

	do_folder_rename(old, new);
}
