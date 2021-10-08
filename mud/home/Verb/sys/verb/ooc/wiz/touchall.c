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
#include <kotaka/paths/verb.h>
#include <status.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Touchall";
}

string *query_help_contents()
{
	return ({ "Calls call_touch on all clones of a given master object" });
}

void main(object actor, mapping roles)
{
	int i, sz, tcount;
	mixed *st;
	string path;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to touch all clones.\n");
		return;
	}

	path = roles["raw"];

	if (!path) {
		send_out("Usage: touchall <path>\n");
		return;
	}

	st = status(path);

	if (!st) {
		send_out("No such object.\n");
		return;
	}

	sz = status(ST_OTABSIZE);

	for (i = 0; i < sz; i++) {
		object obj;

		obj = find_object(path + "#" + i);

		if (obj) {
			call_touch(obj);
			tcount++;
		}
	}

	send_out(tcount + " objects touched.\n");
}
