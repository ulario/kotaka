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
#include <kernel/user.h>
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
	object user, *users;
	string kicker_name;
	string site;
	string args;
	int sz;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can siteban.\n");
		return;
	}

	args = roles["raw"];

	switch(sscanf(args, "%s %s", site, args)) {
	case 0:
		if (args == "") {
			send_out("Usage: siteban <site> <ban message, if any>\n");
			return;
		} else {
			site = args;
			args = nil;
		}
		break;

	case 2:
		break;
	}

	if (sscanf(site, "127.%*s") || site == "::1") {
		send_out("You cannot siteban localhost\n");
		return;
	}

	if (BAND->query_is_site_banned(site)) {
		BAND->ban_site(site, args);
		send_out("Updated siteban message.\n");
	} else {
		BAND->ban_site(site, args);
		send_out("Banned site.\n");
	}
}
