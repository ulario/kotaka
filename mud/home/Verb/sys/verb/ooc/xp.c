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
	return "Xp";
}

string *query_help_contents()
{
	return ({ "Shows your account's experience point total." });
}

void main(object actor, mapping roles)
{
	string name;
	mixed total;

	if (roles["raw"]) {
		send_out("usage: xp\n");
		return;
	}

	name = query_user()->query_name();

	if (!name) {
		send_out("You aren't logged in.\n");
		return;
	}

	total = ACCOUNTD->query_account_property(name, "xp");

	if (total) {
		if (total > 0) {
			send_out("You have " + total + " XP :)\n");
		} else {
			send_out("You are " + -total + " XP in debt! >:(\n");
		}
	} else {
		send_out("You have no XP :(\n");
	}
}
