/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020, 2021  Raymond Jennings
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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;
inherit "~Text/lib/sub";

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Kick";
}

string *query_help_contents()
{
	return ({ "Ejects a player from the mud." });
}

void main(object actor, mapping roles)
{
	object turkey;
	object user;
	string username;
	string kicker_name;
	string turkey_name;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("Only a wizard or an administrator can kick someone.\n");
		return;
	}

	username = roles["raw"];

	if (!username) {
		send_out("Usage: kick <user>\n");
		return;
	}

	if (username == user->query_username()) {
		send_out("You cannot kick yourself.\n");
		return;
	}

	turkey = TEXT_USERD->find_user(username);

	if (!turkey) {
		send_out("That user is not online.\n");
		return;
	}

	if (username == "admin") {
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

	kicker_name = user->query_titled_name();
	turkey_name = turkey->query_titled_name();

	user->message("You kick " + turkey_name + " from the mud.\n");
	send_to_all_except(kicker_name + " kicked " + turkey_name + " from the mud.\n", ({ turkey, query_user() }) );
	turkey->message(kicker_name + " kicked you from the mud.\n");

	turkey->quit("kicked");
}
