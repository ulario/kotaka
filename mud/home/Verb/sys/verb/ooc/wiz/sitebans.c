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
	string *sites;
	object user;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to list sitebans.\n");
		return;
	}

	sites = BAND->query_sitebans();

	if (sizeof(sites)) {
		int i;
		int sz;

		send_out("Banned sites:\n");

		sz = sizeof(sites);

		for (i = 0; i < sz; i++) {
			string site;
			string message;
			mapping ban;
			mixed expire;
			string issuer;
			int remaining;

			site = sites[i];
			ban = BAND->query_siteban(site);

			send_out(site + " ");

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
					send_out("(expires in " + ((remaining + 59) / 60) + " minutes)");
				} else if (remaining < 86400) {
					send_out("(expires in " + ((remaining + 3599) / 3600) + " hours)");
				} else {
					send_out("(expires in " + ((remaining + 86399) / 86400) + " days)");
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
		send_out("There are no banned sites.\n");
	}
}
