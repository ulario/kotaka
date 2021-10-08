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
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>
#include <kotaka/paths/thing.h>

inherit "/lib/string/object";
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Pop";
}

string *query_help_contents()
{
	return ({ "Performs a specifc operation on a property of a given object.", "Does this atomically, and can be useful for numeric properties." });
}

void main(object actor, mapping roles)
{
	mixed opvalue, *pinfo, pvalue;
	object obj, user;
	string args, oname, operatur, pname, *users;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to set object properties.\n");
		return;
	}

	args = roles["raw"];

	if (!args || sscanf(args, "%s %s %s %s", operatur, oname, pname, pvalue) != 4) {
		send_out("Usage: pop <operator> <object> <property> <value>\n");
		return;
	}

	if (!sizeof( ({ operatur })
		& ({ "add", "sub", "mul", "div", "radd", "and", "or", "xor" }) )
	) {
		send_out("Invalid operator.\nPlease use add, sub, mul, div, radd, and, or, or xor.\n");
		return;
	}

	obj = parse_object(oname);

	if (!obj) {
		send_out(oname + ": No such object.\n");
		return;
	}

	pinfo = PROPERTYD->query_property(pname);

	if (!pinfo) {
		send_out(pname + ": No such property.\n");
		return;
	}

	pvalue = PARSER_VALUE->parse(pvalue);
	opvalue = obj->query_property(pname);

	switch(operatur) {
	case "add":
		obj->set_property(pname, opvalue + pvalue);
		break;

	case "and":
		obj->set_property(pname, opvalue & pvalue);
		break;

	case "div":
		obj->set_property(pname, opvalue / pvalue);
		break;

	case "mul":
		obj->set_property(pname, opvalue * pvalue);
		break;

	case "or":
		obj->set_property(pname, opvalue | pvalue);
		break;

	case "radd":
		obj->set_property(pname, pvalue + opvalue);
		break;

	case "sub":
		obj->set_property(pname, opvalue - pvalue);
		break;

	case "xor":
		obj->set_property(pname, opvalue ^ pvalue);
	}

	send_out("Done.\n");
}
