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
#include <status.h>

inherit LIB_VERB;
inherit "~System/lib/struct/list";

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	/* Report all objects inheriting a destructed inheritable */
	mixed **list;
	object proxy;

	if (query_user()->query_class() < 2) {
		send_out("Only a wizard can do that.\n");
		return;
	}

	list = OBJECTD->query_program_indices();
	proxy = PROXYD->get_proxy(query_user()->query_name());

	while (!list_empty(list)) {
		int pindex;
		object pinfo;
		mixed *finfo;
		string *incs;
		int ssz, j;
		int ctime;
		string path;
		string *shinies;
		string *missing;

		pindex = list_front(list);
		list_pop_front(list);

		pinfo = OBJECTD->query_program_info(pindex);

		if (pinfo->query_destructed()) {
			continue;
		}

		path = pinfo->query_path();
		shinies = ({ });
		missing = ({ });

		ctime = status(path, O_COMPILETIME);

		{
			mixed *finfo;

			finfo = proxy->file_info(path + ".c");

			if (!finfo) {
				missing += ({ path + ".c" });
			} else if (finfo[1] > ctime) {
				shinies += ({ path + ".c" });
			}
		}

		incs = pinfo->query_includes();
		ssz = sizeof(incs);

		for (j = 0; j < ssz; j++) {
			string inc;
			mixed *finfo;

			inc = incs[j];
			finfo = proxy->file_info(inc);

			if (!finfo) {
				missing += ({ inc });
			} else if (finfo[1] > ctime) {
				shinies += ({ inc });
			}
		}

		if (sizeof(shinies)) {
			send_out(path + " is older than:\n" + implode(shinies, "\n") + "\n\n");
		}

		if (sizeof(missing)) {
			send_out(path + " is missing:\n" + implode(missing, "\n") + "\n\n");
		}
	}
}
