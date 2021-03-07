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
#include <kotaka/paths/account.h>
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
	string site;
	string kicker_name;
	string turkey_name;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can remove a siteban.\n");
		return;
	}

	site = roles["raw"];

	if (!site) {
		send_out("Usage: unsiteban <site>\n");
		return;
	}

	if (!BAND->query_is_site_banned(site)) {
		send_out("That site is not banned.\n");
		return;
	}

	BAND->unban_site(site);
	send_out("Site unbanned.\n");
}
