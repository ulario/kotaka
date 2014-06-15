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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	object turkey;
	object user;
	string kicker_name;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can nuke someone.\n");
		return;
	}

	if (roles["raw"] == "") {
		send_out("Who do you wish to nuke?\n");
		return;
	}

	if (roles["raw"] == user->query_username()) {
		send_out("You cannot nuke yourself.\n");
		return;
	}

	if (roles["raw"] == "admin") {
		send_out("You cannot nuke admin.\n");
		return;
	}

	if (!ACCOUNTD->query_is_registered(roles["raw"])) {
		send_out("There is no such user.\n");
		return;
	}

	ACCOUNTD->unregister_account(roles["raw"]);

	turkey = TEXT_USERD->find_user(roles["raw"]);
	kicker_name = user->query_titled_name();

	user->message("You nuke " + roles["raw"] + " from the mud.\n");

	TEXT_SUBD->send_to_all_except(roles["raw"] + " has been nuked from the mud by " + kicker_name + "!\n", ({ turkey, query_user() }) );

	if (turkey) {
		turkey->message("You have been nuked from the mud by " + kicker_name + "!\n");
		turkey->quit();
	}
}
