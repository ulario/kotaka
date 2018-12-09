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
	string *sites;
	object user;
	int sz;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to list sitebans.\n");
		return;
	}

	sites = BAND->query_sitebans();
	sz = sizeof(sites);

	if (sz) {
		int i;
		int time;
		string **table;

		table = allocate(sz + 1);
		table[0] = ({ "Mask", "Issuer", "Exp", "Message" });

		time = time();

		for (i = 0; i < sz; i++) {
			string mask;
			mapping ban;
			string issuer;
			mixed expire;
			mixed remaining;
			string message;

			mask = sites[i];

			ban = BAND->query_siteban(mask);

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

			table[i + 1] = ({ mask, issuer, remaining, message });
		}

		send_out(render_table(table, 2) + "\n");
	} else {
		send_out("There are no banned sites.\n");
	}
}
