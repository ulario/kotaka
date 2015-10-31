/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <status.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	string path;
	mixed *st;
	int i, sz;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to do a clone check.\n");
		return;
	}

	path = roles["raw"];

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
			send_out(object_name(obj) + "\n");
		}
	}
}
