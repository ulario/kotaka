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
	int i, sz;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can siteban.\n");
		return;
	}

	site = roles["raw"];

	if (site == "") {
		send_out("What do you wish to siteban?\n");
		return;
	}

	if (site == "127.0.0.1" || site == "::1") {
		send_out("You cannot siteban localhost\n");
		return;
	}

	if (BAND->query_is_site_banned(site)) {
		send_out("That site is already banned.\n");
		return;
	}

	BAND->ban_site(site);
	users = TEXT_USERD->query_users();
	users += TEXT_USERD->query_guests();
	sz = sizeof(users);

	/* kick turkeys */

	kicker_name = TEXT_SUBD->titled_name(user->query_username(), user->query_class());

	for (i = 0; i < sz; i++) {
		object conn;
		object turkey;
		string turkey_name;

		turkey = users[i];
		conn = turkey;

		while (conn && conn <- LIB_USER) {
			conn = conn->query_conn();
		}

		if (!conn) {
			continue;
		}

		if (!TEXT_USERD->is_sitebanned(query_ip_number(conn))) {
			continue;
		}

		turkey_name = TEXT_SUBD->titled_name(turkey->query_name(), TEXT_SUBD->query_user_class(turkey->query_name()));

		user->message("You siteban " + turkey_name + " from the mud.\n");

		TEXT_SUBD->send_to_all_except(kicker_name + " sitebans " + turkey_name + " from the mud.\n", ({ turkey, query_user() }) );

		turkey->message(kicker_name + " sitebans you from the mud.\n");
		turkey->quit();
	}
}
