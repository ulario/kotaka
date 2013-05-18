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
#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

void main(object actor, mapping roles)
{
	string *users;
	object user;
	object obj;
	string oname;
	string pname;
	mixed pvalue;
	string args;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to set object properties.\n");
		return;
	}

	if (sscanf(args, "%s %s %s", oname, pname, pvalue) != 3) {
		send_out("Usage: lpset <object> <local property name> <value>\n");
		return;
	}

	obj = find_object(oname);

	if (!obj) {
		obj = CATALOGD->lookup_object(oname);
	}

	if (!obj) {
		send_out(oname + ": No such object.\n");
		return;
	}

	pvalue = PARSE_VALUE->parse(pvalue);

	obj->set_local_property(pname, pvalue);

	send_out("Done.\n");
}
