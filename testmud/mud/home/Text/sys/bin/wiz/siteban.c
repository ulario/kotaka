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

inherit LIB_BIN;

void main(string args)
{
	object turkey;
	object user;
	string kicker_name;
	string turkey_name;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can siteban.\n");
		return;
	}

	if (args == "") {
		send_out("What do you wish to siteban?\n");
		return;
	}

	if (args == "127.0.0.1" || args == "::1") {
		send_out("You cannot siteban localhost\n");
		return;
	}

	if (BAND->query_is_site_banned(args)) {
		send_out("That site is already banned.\n");
		return;
	}

	BAND->ban_site(args);

	/* kick turkeys */
}
