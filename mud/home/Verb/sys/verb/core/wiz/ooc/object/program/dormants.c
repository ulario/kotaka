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
inherit "~System/lib/struct/list";

string *query_parse_methods()
{
	return ({ "raw" });
}

/* ({ ({ file names }), ({ file sizes }), ({ file mod times }), ({ objects }) }) */

private void list_dormants(mixed **list, object proxy, string dir)
{
	string *names;
	int *sizes;
	mixed *times;
	mixed *objs;
	int sz;

	if (dir == "/") {
		dir = "";
	}

	({ names, sizes, times, objs }) = proxy->get_dir(dir + "/*");

	for (sz = sizeof(names); --sz >= 0; ) {
		if (sizes[sz] == -2) {
			if (names[sz] != "lib") {
				list_dormants(list, proxy, dir + "/" + names[sz]);
			}

			continue;
		}

		if (!objs[sz]) {
			list_push_back(list, dir + "/" + names[sz]);
		}
	}
}

void main(object actor, mapping roles)
{
	mixed **list;
	object proxy;

	if (query_user()->query_class() < 3) {
		send_out("You do not have sufficient access rights to list dormants.\n");
		return;
	}

	list = ({ nil, nil });
	proxy = PROXYD->get_proxy(query_user()->query_name());

	list_dormants(list, proxy, "/");

	if (list_empty(list)) {
		send_out("There are no dormant LPC source files.\n");
		return;
	}

	send_out("These LPC files are not compiled:\n");

	while (!list_empty(list)) {
		send_out(list_front(list) + "\n");
		list_pop_front(list);
	}
}
