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
#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>
#include <kotaka/paths/utility.h>
#include <type.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	string path;
	mixed **list;
	object proxy;
	mixed **compile;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to reinherit.\n");
		return;
	}

	path = roles["raw"];
	list = OBJECTD->query_program_indices();
	proxy = PROXYD->get_proxy(query_user()->query_name());

	compile = ({ nil, nil });

	while (!list_empty(list)) {
		object pinfo;

		pinfo = list_front(list);
		list_pop_front(list);

		if (!pinfo || pinfo->query_destructed()) {
			continue;
		}

		if (sizeof(pinfo->query_includes() & ({ path }) )) {
			string ppath;

			ppath = pinfo->query_path();

			if (sscanf(ppath, "%*s" + INHERITABLE_SUBDIR + "%*s")) {
				proxy->destruct_object(ppath);
			} else {
				list_push_back(compile, ppath);
			}
		}
	}

	while (!list_empty(compile)) {
		string ppath;

		ppath = list_front(compile);
		list_pop_front(compile);

		proxy->compile_object(ppath);
	}
}
