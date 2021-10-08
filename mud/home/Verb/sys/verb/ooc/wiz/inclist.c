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
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>
#include <kotaka/assert.h>

inherit LIB_VERB;
inherit "~System/lib/struct/list";

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Inclist";
}

string *query_help_contents()
{
	return ({ "Lists all programs including a particular file" });
}

void main(object actor, mapping roles)
{
	/* Report all objects inheriting a destructed inheritable */
	mixed **list;
	string path;

	if (query_user()->query_class() < 2) {
		send_out("Only a wizard can do that.\n");
		return;
	}

	path = roles["raw"];

	if (!path) {
		send_out("Usage: inclist <path>\n");
		return;
	}

	list = OBJECTD->query_program_indices();

	while (!list_empty(list)) {
		int pindex;
		object pinfo;
		string *inc;

		pindex = list_front(list);
		list_pop_front(list);

		pinfo = OBJECTD->query_program_info(pindex);

		if (pinfo->query_destructed()) {
			continue;
		}

		inc = pinfo->query_includes();
		ASSERT(inc);

		if (sizeof(inc & ({ path }))) {
			send_out(pinfo->query_path() + " includes " + path + "\n");
		}
	}
}
