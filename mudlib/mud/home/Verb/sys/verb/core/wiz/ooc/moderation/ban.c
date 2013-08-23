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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

void main(object actor, mapping roles)
{
	object turkey;
	object user;
	string kicker_name;
	string turkey_name;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("Only a wizard or an administrator can ban someone.\n");
		return;
	}

	if (roles["raw"] == "") {
		send_out("Who do you wish to ban?\n");
		return;
	}

	if (roles["raw"] == user->query_username()) {
		send_out("You cannot ban yourself.\n");
		return;
	}

	if (roles["raw"] == "admin") {
		send_out("You cannot ban admin.\n");
		return;
	}

	if (BAND->query_is_username_banned(roles["raw"])) {
		send_out("That user is already banned.\n");
		return;
	}

	switch(TEXT_SUBD->query_user_class(roles["raw"])) {
	case 3: /* administrator.  Only the mud owner can ban them */
		if (user->query_username() != "admin") {
			send_out("Only the mud owner can ban an administrator.");
			return;
		}
		break;
	case 2: /* wizard.  Only an administrator can ban them */
		if (user->query_class() < 3) {
			send_out("Only an administrator can ban a wizard.");
			return;
		}
		break;
	default:
		if (user->query_class() < 2) {
			send_out("Only a wizard can ban someone.");
			return;
		}
		break;
	}

	BAND->ban_username(roles["raw"]);

	kicker_name = TEXT_SUBD->titled_name(user->query_username(), user->query_class());
	turkey_name = TEXT_SUBD->titled_name(roles["raw"], TEXT_SUBD->query_user_class(roles["raw"]));

	user->message("You ban " + turkey_name + " from the mud.\n");
	turkey = TEXT_USERD->find_user(roles["raw"]);

	TEXT_SUBD->send_to_all_except(kicker_name + " bans " + turkey_name + " from the mud.\n", ({ turkey, query_user() }) );

	if (turkey) {
		turkey->message(kicker_name + " bans you from the mud.\n");
		turkey->quit();
	}
}
