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
#include <kotaka/privilege.h>
#include <kotaka/paths/system.h>
#include <status.h>

inherit "call_guard";

static void call_touch(object obj)
{
	if (previous_program() == TOUCHD) {
		::call_touch(obj);
	} else {
		TOUCHD->call_touch(obj);
	}
}

static void touch()
{
}

nomask void _F_touch(string function)
{
	int index;
	int sz;
	object pinfo;
	object programd;
	string base;
	string toucher;
	string *touchers;
	string oname;

	ACCESS_CHECK(previous_program() == OBJECTD);

	oname = object_name(this_object());
	base = oname;
	sscanf(base, "%s#%*d", base);

	programd = find_object(PROGRAMD);

	if (!programd) {
		return;
	}

	pinfo = programd->query_program_info(status(this_object(), O_INDEX));

	if (!pinfo) {
		return;
	}

	touchers = pinfo->query_inherited_touchers();
	toucher = pinfo->query_toucher();

	sz = sizeof(touchers);

	for (index = 0; index < sz; index++) {
		call_other(this_object(), touchers[index]);
	}

	if (toucher) {
		call_other(this_object(), toucher);
	}

	touch();
}
