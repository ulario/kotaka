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

inherit "/lib/string/sprint";
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Lpget";
}

string *query_help_contents()
{
	return ({ "Queries a local property of a given object" });
}

void main(object actor, mapping roles)
{
	object obj, user;
	string args, oname, pname, *users;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to get object properties.\n");
		return;
	}

	args = roles["raw"];

	if (!args || !sscanf(args, "%s %s", oname, pname)) {
		send_out("Usage: lpget <object> <property>\n");
		return;
	}

	obj = find_object(oname);

	if (!obj) {
		obj = IDD->find_object_by_name(oname);
	}

	if (!obj) {
		send_out(oname + ": No such object.\n");
		return;
	}

	send_out(hybrid_sprint(obj->query_local_property(pname)) + "\n");
}
