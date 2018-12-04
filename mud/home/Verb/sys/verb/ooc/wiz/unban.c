/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	object user;
	string username;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("Only a wizard or an administrator can unban someone.\n");
		return;
	}

	username = roles["raw"];

	if (username == "") {
		send_out("Who do you wish to unban?\n");
		return;
	}

	if (!BAND->query_is_user_banned(username)) {
		send_out("That user is not banned.\n");
		return;
	}

	switch(TEXT_SUBD->query_user_class(username)) {
	case 3: /* administrator.  Only the mud owner can ban them */
		if (user->query_username() != "admin") {
			send_out("Only the mud owner can unban an administrator.\n");
			return;
		}
		break;

	case 2: /* wizard.  Only an administrator can ban them */
		if (user->query_class() < 3) {
			send_out("Only an administrator can unban a wizard.\n");
			return;
		}
		break;

	default:
		if (user->query_class() < 2) {
			send_out("Only a wizard can unban someone.\n");
			return;
		}
		break;
	}

	if (!BAND->query_is_user_banned(username)) {
		send_out("User not banned.\n");
		return;
	}

	BAND->unban_user(username);

	send_out("User unbanned.\n");
}
