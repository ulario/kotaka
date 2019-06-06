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
#include <status.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>

inherit LIB_VERB;
inherit "/lib/sort";
inherit "~System/lib/struct/list";

string *query_parse_methods()
{
	return ({ "raw" });
}

/*

an object is stale if:

* it inherits a destructed library
* any of its sources either don't exist or were modified after the object was compiled

*/

static void report_tick(mixed **list, object user, object proxy)
{
	int pindex;
	object pinfo;
	mixed *libs;
	string *libnames;
	string *incs;
	int sz;
	int ctime;
	string path;
	mapping srcstat;
	mixed *info;

	pindex = list_front(list);
	list_pop_front(list);

	if (!list_empty(list)) {
		call_out("report_tick", 0, list, user, proxy);
	}

	pinfo = OBJECTD->query_program_info(pindex);
	path = pinfo->query_path();

	if (pinfo->query_destructed()) {
		return;
	}

	libs = pinfo->query_inherits();
	libnames = ({ });
	ASSERT(libs);

	for (sz = sizeof(libs); --sz >= 0; ) {
		object pinfo2;

		pinfo2 = OBJECTD->query_program_info(libs[sz]);

		if (pinfo2->query_destructed()) {
			libnames += ({ pinfo2->query_path() });
		}
	}

	ctime = status(path, O_COMPILETIME);

	srcstat = ([ ]);
	/* 1 = modified */
	/* -1 = missing */

	info = proxy->file_info(path + ".c");

	if (!info) {
		srcstat[path + ".c"] = -1;
	} else if (info[1] > ctime) {
		srcstat[path + ".c"] = 1;
	}

	incs = pinfo->query_includes();

	for (sz = sizeof(incs); --sz >= 0; ) {
		string inc;

		inc = incs[sz];

		info = proxy->file_info(inc);

		if (!info) {
			srcstat[inc] = -1;
		} else if (info[1] > ctime) {
			srcstat[inc] = 1;
		}
	}

	if (map_sizeof(srcstat) || sizeof(libnames)) {
		int sz;
		int i;

		user->message("\n" + path + " is stale\n");

		qsort(libnames, 0, sizeof(libnames));

		if (sz = sizeof(libnames)) {
			user->message("  Destructed inherits\n");

			for (; i < sz; i++) {
				user->message("    " + libnames[i] + "\n");
			}

			if (map_sizeof(srcstat)) {
				user->message("\n");
			}
		}

		if (map_sizeof(srcstat)) {
			user->message("  Missing or modified sources\n");
		}

		switch (srcstat[path + ".c"]) {
		case -1:
			user->message("    " + path + ".c (missing)\n");
			break;
		case 1:
			user->message("    " + path + ".c (modified)\n");
		}

		srcstat[path + ".c"] = nil;

		if (sz = map_sizeof(srcstat)) {
			string *paths;
			int *stats;
			int i;

			paths = map_indices(srcstat);
			stats = map_values(srcstat);

			for (i = 0; i < sz; i++) {
				user->message("  " + paths[i]);

				switch(stats[i]) {
				case -1:
					user->message(" is missing\n");
					break;
				case 1:
					user->message(" was modified\n");
					break;
				}
			}

			user->message("\n");
		}
	}
}

void main(object actor, mapping roles)
{
	mixed **list;
	object proxy;
	object user;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("Only a wizard can do that.\n");
		return;
	}

	list = OBJECTD->query_program_indices();
	proxy = PROXYD->get_proxy(query_user()->query_name());

	if (!list_empty(list)) {
		call_out("report_tick", 0, list, user, proxy);
	}
}
