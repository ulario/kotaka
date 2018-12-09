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
#include <kotaka/paths/account.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;
inherit "/lib/string/format";

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	string *users;
	object user;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to list bans.\n");
		return;
	}

	users = BAND->query_bans();

	if (sizeof(users)) {
		int i;
		int sz;
		int time;
		string **table;

		sz = sizeof(users);

		table = allocate(sz + 1);
		table[0] = ({ "User", "Issuer", "Expire", "Message" });

		time = time();

		for (i = 0; i < sz; i++) {
			string username;
			mapping ban;
			mixed remaining;
			string issuer;
			mixed expire;
			string message;

			username = users[i];
			ban = BAND->query_ban(username);

			issuer = ban["issuer"];

			if (!issuer) {
				issuer = "";
			}

			expire = ban["expire"];

			if (expire == nil) {
				remaining = "forever";
			} else {
				remaining = expire - time;

				if (remaining < 60) {
					remaining = remaining + "s";
				} else if (remaining < 3600) {
					remaining = ((remaining + 59) / 60) + "m";
				} else if (remaining < 86400) {
					remaining = ((remaining + 3599) / 3600) + "h";
				} else {
					remaining = ((remaining + 86399) / 86400) + "d";
				}
			}

			message = ban["message"];

			if (!message) {
				message = "";
			}

			table[i + 1] = ({ username, issuer, remaining, message });
		}

		send_out(render_table(table, 2) + "\n");
	} else {
		send_out("There are no banned users.\n");
	}
}
