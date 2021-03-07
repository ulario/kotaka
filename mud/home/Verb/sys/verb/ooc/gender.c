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
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Gender";
}

string *query_help_contents()
{
	return ({ "Changes your OOC gender." });
}

void main(object actor, mapping roles)
{
	string name, gender;

	name = query_user()->query_name();

	if (!name) {
		send_out("You aren't logged in.\n");
		return;
	}

	gender = roles["raw"];

	if (!gender) {
		switch(ACCOUNTD->query_account_property(name, "gender")) {
		case nil:
			send_out("You have no gender.\n");
			break;

		case "male":
			send_out("You are male.\n");
			break;

		case "female":
			send_out("You are female.\n");
			break;
		}
	} else {
		switch(gender) {
		case "male":
			ACCOUNTD->set_account_property(name, "gender", "male");
			send_out("You are now male.\n");
			break;

		case "female":
			ACCOUNTD->set_account_property(name, "gender", "female");
			send_out("You are now female.\n");
			break;

		case "none":
			ACCOUNTD->set_account_property(name, "gender", nil);
			send_out("You are now neutral.\n");
			break;

		default:
			send_out("Male, female, or none please.\n");
		}
	}
}
