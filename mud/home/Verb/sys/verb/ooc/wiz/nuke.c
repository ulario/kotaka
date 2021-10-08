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
#include <kotaka/paths/account.h>
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
	return "Nuke";
}

string *query_help_contents()
{
	return ({ "Deletes a player's account" });
}

void main(object actor, mapping roles)
{
	object turkey, user;
	string args, kicker, username;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can nuke someone.\n");
		return;
	}

	args = roles["raw"];

	if (!args) {
		send_out("Usage: nuke <user>\n");
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

	turkey = TEXT_USERD->find_user(args);
	kicker = user->query_titled_name();

	if (turkey) {
		turkey->message("You have been nuked from the mud by " + kicker + "!\n");
		turkey->quit("nuked");
	}

	ACCOUNTD->unregister_account(args);

	user->message("You nuke " + args + " from the mud!\n");
	send_to_all_except(args + " has been nuked from the mud by " + kicker + "!\n", ({ turkey, query_user() }) );
}
