/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014  Raymond Jennings
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

void main(object actor, mapping roles)
{
	object user;
	string name;
	string pass;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can change a password.\n");
		return;
	}

	if (sscanf(roles["raw"], "%s %s", name, pass) != 2) {
		send_out("Usage: setpass <account name> <new password>\n");
		return;
	}

	if (!ACCOUNTD->query_is_registered(name)) {
		send_out("There is no such user.\n");
		return;
	}

	ACCOUNTD->change_password(name, pass);
}
