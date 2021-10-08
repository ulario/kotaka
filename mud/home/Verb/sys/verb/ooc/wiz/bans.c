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
inherit "~/lib/banlist";

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Bans";
}

string *query_help_contents()
{
	return ({ "Lists all bans on record" });
}

void main(object actor, mapping roles)
{
	string *users;
	object user;
	int sz;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to list bans.\n");
		return;
	}

	if (roles["raw"]) {
		send_out("Usage: bans\n");
		return;
	}

	users = BAND->query_bans();
	sz = sizeof(users);

	if (sz) {
		mapping *bans;
		int i;

		bans = allocate(sz);

		for (i = 0; i < sz; i++) {
			mapping ban;

			ban = BAND->query_ban(users[i]);

			if (!ban) {
				users[i] = nil;
				continue;
			}

			bans[i] = ban;
		}

		users -= ({ nil });
		bans -= ({ nil });

		send_out(print_bans("User", users, bans));

		send_out("\n");
	} else {
		send_out("There are no banned users.\n");
	}
}
