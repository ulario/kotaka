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
#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

int query_raw()
{
	return 1;
}

void main(object actor, mixed *tree)
{
	object turkey;
	object user;
	string kicker_name;
	string args;

	args = fetch_raw(tree);

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can nuke someone.\n");
		return;
	}

	if (args == "") {
		send_out("Who do you wish to nuke?\n");
		return;
	}

	if (args == user->query_username()) {
		send_out("You cannot nuke yourself.\n");
		return;
	}

	if (args == "admin") {
		send_out("You cannot nuke admin.\n");
		return;
	}

	if (!ACCOUNTD->query_is_registered(args)) {
		send_out("There is no such user.\n");
		return;
	}

	ACCOUNTD->unregister_account(args);

	turkey = TEXT_USERD->find_user(args);
	kicker_name = TEXT_SUBD->titled_name(user->query_username(), user->query_class());

	user->message("You nuke " + args + " from the mud.\n");

	TEXT_SUBD->send_to_all_except(args + " has been nuked from the mud by " + kicker_name + "!\n", ({ turkey, query_user() }) );

	if (turkey) {
		turkey->message("You have been nuked from the mud by " + kicker_name + "!\n");
		turkey->quit();
	}
}
