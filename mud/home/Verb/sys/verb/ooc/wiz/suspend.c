/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C)  2021  Raymond Jennings
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
inherit "/lib/string/case";
inherit "/lib/string/sprint";
inherit "~Text/lib/sub";

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	object user;

	string args;
	string message, unit, time;
	string username;

	object turkey;

	int duration, value;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("Only a wizard can suspend someone.\n");
		return;
	}

	args = roles["raw"];

	if (!args) {
		send_out("Usage: suspend <username> <duration> [<message>]\n");
		return;
	}

	if (!sscanf(args, "%s %s", username, args)) {
		send_out("Usage: suspend <username> <duration> [<message>]\n");
		return;
	}

	if (!sscanf(args, "%s %s", time, message)) {
		time = args;
		message = nil;
	}

	if (!sscanf(time, "%d%s", value, unit)) {
		send_out("Duration must be <number><unit>\n");
		send_out("Valid units:\n");
		send_out("s - seconds\n");
		send_out("m - minutes\n");
		send_out("h - hours\n");
		send_out("d - days\n");
		send_out("w - weeks\n");
		send_out("mo - months\n");
		send_out("y - years\n");
		return;
	}

	switch(unit) {
	case "s":
		duration = value;
		break;

	case "m":
		duration = value * 60;
		break;

	case "h":
		duration = value * 3600;
		break;

	case "d":
		duration = value * 86400;
		break;

	case "w":
		duration = value * 7 * 86400;
		break;

	case "mo":
		duration = value * 30 * 86400;
		break;

	case "y":
		duration = value * 365 * 86400;
		break;

	default:
		send_out("Invalid unit\n");
		send_out("Valid units:\n");
		send_out("s - seconds\n");
		send_out("m - minutes\n");
		send_out("h - hours\n");
		send_out("d - days\n");
		send_out("w - weeks\n");
		send_out("mo - months\n");
		send_out("y - years\n");
	}

	if (username == user->query_username()) {
		send_out("You cannot suspend yourself.\n");
		return;
	}

	if (username == "admin") {
		send_out("You cannot suspend admin.\n");
		return;
	}

	switch(query_user_class(username)) {
	case 3: /* administrator.  Only the mud owner can suspend them */
		if (user->query_username() != "admin") {
			send_out("Only the mud owner can suspend an administrator.\n");
			return;
		}
		break;

	case 2: /* wizard.  Only an administrator can suspend them */
		if (user->query_class() < 3) {
			send_out("Only an administrator can suspend a wizard.\n");
			return;
		}
		break;

	default:
		if (user->query_class() < 2) {
			send_out("Only a wizard can suspend someone.\n");
			return;
		}
		break;
	}

	username = to_lower(username);

	BAND->ban_user(username,
		([
			"expire" : duration ? time() + duration : nil,
			"issuer": user->query_username(),
			"message": message
		])
	);

	turkey = TEXT_USERD->find_user(username);

	if (turkey) {
		string kicker_name;
		string turkey_name;

		kicker_name = user->query_titled_name();
		turkey_name = query_titled_name(username);

		send_to_all_except(kicker_name + " suspends " + turkey_name + " from the mud!\n", ({ turkey, query_user() }) );

		turkey->message(kicker_name + " suspends you from the mud!\n");
		turkey->quit("banned");
	}

	user->message("User suspended.\n");
}
