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
#include <kotaka/paths/account.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/text.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	string *users;
	object user;
	object obj;
	string username;
	string pname;
	string operatur;
	mixed pvalue;
	mixed opvalue;
	mixed *pinfo;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to set account properties.\n");
		return;
	}

	if (sscanf(args, "%s %s %s %s", operatur, username, pname, pvalue) != 4) {
		send_out("Usage: apop <operator> <user name> <property name> <value>\n");
		return;
	}

	if (!sizeof( ({ operatur })
		& ({ "add", "sub", "mul", "div", "radd", "and", "or", "xor" }) )
	) {
		send_out("Invalid operator.\nPlease use add, sub, mul, div, radd, and, or, or xor.\n");
		return;
	}

	if (!ACCOUNTD->query_is_registered(username)) {
		send_out("There is no such user.\n");
		return;
	}

	pvalue = PARSE_VALUE->parse(pvalue);
	opvalue = ACCOUNTD->query_account_property(username, pname);

	switch(operatur) {
	case "add":
		ACCOUNTD->set_account_property(username, pname, opvalue + pvalue);
		break;
	case "and":
		ACCOUNTD->set_account_property(username, pname, opvalue & pvalue);
		break;
	case "div":
		ACCOUNTD->set_account_property(username, pname, opvalue / pvalue);
		break;
	case "mul":
		ACCOUNTD->set_account_property(username, pname, opvalue * pvalue);
		break;
	case "or":
		ACCOUNTD->set_account_property(username, pname, opvalue | pvalue);
		break;
	case "radd":
		ACCOUNTD->set_account_property(username, pname, pvalue + opvalue);
		break;
	case "sub":
		ACCOUNTD->set_account_property(username, pname, opvalue - pvalue);
		break;
	case "xor":
		ACCOUNTD->set_account_property(username, pname, opvalue ^ pvalue);
		break;
	}

	send_out("Done.\n");
}
