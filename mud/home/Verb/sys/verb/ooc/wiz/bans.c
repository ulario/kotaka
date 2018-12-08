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

		send_out("Banned users:\n");

		sz = sizeof(users);

		for (i = 0; i < sz; i++) {
			string username;
			string message;
			mapping ban;
			mixed expire;
			mixed issuer;
			int remaining;

			username = users[i];
			ban = BAND->query_ban(username);

			send_out(username + " ");

			issuer = ban["issuer"];

			if (issuer) {
				send_out("(issued by " + issuer + ") ");
			}

			expire = ban["expire"];

			if (expire == nil) {
				send_out("(permanent)");
			} else {
				remaining = expire - time();

				if (remaining < 60) {
					send_out("(expires in " + remaining + " seconds)");
				} else if (remaining < 3600) {
					send_out("(expires in " + (remaining / 60 + 1) + " minutes)");
				} else if (remaining < 86400) {
					send_out("(expires in " + (remaining / 3600 + 1) + " hours)");
				} else {
					send_out("(expires in " + (remaining / 86400 + 1) + " days)");
				}
			}

			message = ban["message"];

			if (message) {
				send_out(": " + message + "\n");
			} else {
				send_out(" (no message)\n");
			}
		}
	} else {
		send_out("There are no banned users.\n");
	}
}
