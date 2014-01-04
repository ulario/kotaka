/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
	mixed *st;
	object cinfo;
	int oindex;
	int count;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to do a clone check.\n");
		return;
	}

	st = status(roles["raw"]);

	if (!st) {
		send_out("No such object.\n");
		return;
	}

	cinfo = CLONED->query_clone_info(st[O_INDEX]);

	if (!cinfo) {
		send_out("There are no clones.\n");
		return;
	}

	send_out("There are " + cinfo->query_clone_count() + " clones.\n");

	if (cinfo->query_clone_count() < 20) {
		object *clones;

		clones = cinfo->query_clones();

		if (clones) {
			int sz, i;
			sz = sizeof(clones);

			for (i = 0; i < sz; i++) {
				send_out(object_name(clones[i]) + "\n");
			}
		} else {
			send_out("There were too many to list at one point.\n");
		}
	} else {
		send_out("There are too many to list.\n");
	}
}
