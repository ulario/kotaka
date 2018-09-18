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

	string args;
	string username;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("Only a wizard or an administrator can ban someone.\n");
		return;
	}

	args = roles["raw"];

	switch(sscanf(args, "%s %s", username, args)) {
	case 0:
		if (args == "") {
			send_out("Usage: ban <username> <ban message, if any>\n");
			return;
		} else {
			username = args;
			args = nil;
		}
		break;

	case 2:
		break;
	}

	if (username == user->query_username()) {
		send_out("You cannot ban yourself.\n");
		return;
	}

	if (username == "admin") {
		send_out("You cannot ban admin.\n");
		return;
	}

	switch(TEXT_SUBD->query_user_class(username)) {
	case 3: /* administrator.  Only the mud owner can ban them */
		if (user->query_username() != "admin") {
			send_out("Only the mud owner can ban an administrator.\n");
			return;
		}
		break;

	case 2: /* wizard.  Only an administrator can ban them */
		if (user->query_class() < 3) {
			send_out("Only an administrator can ban a wizard.\n");
			return;
		}
		break;

	default:
		if (user->query_class() < 2) {
			send_out("Only a wizard can ban someone.\n");
			return;
		}
		break;
	}

	if (BAND->query_is_user_banned(username)) {
		BAND->ban_user(username, args, -1);

		send_out("Ban message updated.\n");
	} else {
		object turkey;
		string kicker_name;
		string turkey_name;

		BAND->ban_user(username, args, -1);

		kicker_name = user->query_titled_name();
		turkey_name = TEXT_SUBD->query_titled_name(username);

		turkey = TEXT_USERD->find_user(username);

		TEXT_SUBD->send_to_all_except(kicker_name + " bans " + turkey_name + " from the mud.\n", ({ turkey, query_user() }) );

		if (turkey) {
			turkey->message(kicker_name + " bans you from the mud.\n");
			turkey->quit();
		}

		user->message("You ban " + turkey_name + " from the mud.\n");
	}
}
