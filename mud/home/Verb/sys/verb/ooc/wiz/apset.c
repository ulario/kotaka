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
#include <kotaka/paths/account.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Apset";
}

string *query_help_contents()
{
	return ({ "Sets an account property" });
}

void main(object actor, mapping roles)
{
	mixed *pinfo, pvalue;
	object obj, user;
	string args, pname, username, *users;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to set account properties.\n");
		return;
	}

	args = roles["raw"];

	if (!args || sscanf(args, "%s %s %s", username, pname, pvalue) != 3) {
		send_out("Usage: apset <user name> <property name> <value>\n");
		return;
	}

	if (!ACCOUNTD->query_is_registered(username)) {
		send_out("There is no such user.\n");
		return;
	}

	pvalue = PARSER_VALUE->parse(pvalue);

	ACCOUNTD->set_account_property(username, pname, pvalue);
	send_out("Done.\n");
}
