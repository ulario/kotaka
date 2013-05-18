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
#include <text/paths.h>
#include <status.h>

inherit LIB_WIZVERB;

mixed **query_roles()
{
	return ({ });
}

void main(object actor, mapping roles)
{
	mixed *st;
	object cinfo;
	object first;
	object clone;
	string args;
	int oindex;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to do a clone check.\n");
		return;
	}

	st = status(args);

	if (!st) {
		send_out("No such object.\n");
		return;
	}

	cinfo = CLONED->query_clone_info(st[O_INDEX]);

	send_out("There are " + cinfo->query_clone_count() + " clones.\n");

	first = cinfo->query_first_clone();

	if (first) {
		object clone;

		clone = first;

		do {
			send_out(object_name(clone) + "\n");
			clone = clone->query_next_clone();
		} while (clone != first);
	}
}
