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
#include <kotaka/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	object turkey;
	object user;
	string kicker_name;
	string turkey_name;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("Only a wizard or an administrator can kick someone.\n");
		return;
	}

	if (args == "") {
		send_out("Who do you wish to kick?\n");
		return;
	}

	if (args == user->query_username()) {
		send_out("You cannot kick yourself.\n");
		return;
	}

	turkey = TEXT_USERD->find_user(args);

	if (!turkey) {
		send_out("That user is not online.\n");
		return;
	}

	if (args == "admin") {
		send_out("You cannot kick admin.\n");
		return;
	}

	switch(turkey->query_class()) {
	case 3: /* administrator.  Only the mud owner can ban them */
		if (user->query_username() != "admin") {
			send_out("Only the mud owner can kick an administrator.");
			return;
		}
		break;
	case 2: /* wizard.  Only an administrator can ban them */
		if (user->query_class() < 3) {
			send_out("Only an administrator can kick a wizard.");
			return;
		}
		break;
	default:
		if (user->query_class() < 2) {
			send_out("Only a wizard can kick someone.");
			return;
		}
		break;
	}

	kicker_name = TEXT_SUBD->titled_name(user->query_username(), user->query_class());
	turkey_name = TEXT_SUBD->titled_name(turkey->query_username(), turkey->query_class());

	user->message("You kick " + turkey_name + " from the mud.\n");
	TEXT_SUBD->send_to_all_except(kicker_name + " kicked " + turkey_name + " from the mud.\n", ({ turkey, query_user() }) );
	turkey->message(kicker_name + " kicked you from the mud.\n");

	turkey->quit();
}
