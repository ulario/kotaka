/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2021  Raymond Jennings
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
#include <kernel/user.h>
#include <kotaka/paths/account.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Sitesuspend";
}

string *query_help_contents()
{
	return ({ "Temporarily bans a given IP address or range" });
}

void main(object actor, mapping roles)
{
	object user, *users;
	string args, message, site, time, unit;
	int value, duration;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can sitesuspend.\n");
		return;
	}

	args = roles["raw"];

	if (!args) {
		send_out("Usage: sitesuspend <site> <duration> [<message>]\n");
		return;
	}

	if (!sscanf(args, "%s %s", site, args)) {
		send_out("Usage: sitesuspend <site> <duration> [<message>]\n");
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

	if (sscanf(site, "127.%*s") || site == "::1") {
		send_out("You cannot sitesuspend localhost\n");
		return;
	}

	BAND->ban_site(site,
		([
			"expire": duration ? time() + duration : nil,
			"issuer": user->query_username(),
			"message" : message
		])
	);

	send_out("Site suspended.\n");
}
