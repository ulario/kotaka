/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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
	return "Siteban";
}

string *query_help_contents()
{
	return ({ "Sitebans a given IP address or range" });
}

void main(object actor, mapping roles)
{
	object user, *users;
	string args, message, site;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can siteban.\n");
		return;
	}

	args = roles["raw"];

	if (!args) {
		send_out("Usage: siteban <site> [<message>]\n");
		return;
	}

	if (!sscanf(args, "%s %s", site, message)) {
		site = args;
		message = nil;
	}

	if (sscanf(site, "127.%*s") || site == "::1") {
		send_out("You cannot siteban localhost\n");
		return;
	}

	BAND->ban_site(site, ([ "issuer": user->query_username(), "message" : message ]) );
	send_out("Site banned.\n");
}
