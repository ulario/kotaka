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

	user = query_user();

	if (user->query_class() < 1) {
		send_out("You do not have sufficient access rights to toggle invisibility.\n");
		return;
	}

	switch(roles["raw"])
	{
	case "on":
		ACCOUNTD->set_account_property(user->query_username(), "invisible", 1);
		send_out("You are now invisible.\n");
		break;

	case "off":
		ACCOUNTD->set_account_property(user->query_username(), "invisible", nil);
		send_out("You are now not invisible.\n");
		break;

	default:
		if (ACCOUNTD->query_account_property(user->query_username(), "invisible")) {
			send_out("You are invisible.\n");
		} else {
			send_out("You are not invisible.\n");
		}
	}
}
