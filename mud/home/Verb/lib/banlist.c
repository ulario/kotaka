/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2021  Raymond Jennings
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

inherit "/lib/time";
inherit "/lib/string/format";
inherit "~System/lib/string/align";

static string print_bans(string header, string *targets, mixed *bans)
{
	int sz, i;
	int time;
	string **table;
	int *len;
	string buffer;

	sz = sizeof(bans);

	table = allocate(sz);
	len = ({ strlen(header), 6, 3, 7 });

	time = time();

	buffer = "";

	for (i = 0; i < sz; i++) {
		string target;
		mapping ban;

		string issuer;
		mixed expire;
		mixed remaining;
		string message;
		int sz2;

		target = targets[i];
		ban = bans[i];

		issuer = ban["issuer"];

		if (!issuer) {
			issuer = "";
		}

		expire = ban["expire"];

		if (expire == nil) {
			remaining = "Perm";
		} else {
			remaining = expire - time;

			remaining = timedesc(remaining);
		}

		message = ban["message"];

		if (!message) {
			message = "";
		}

		table[i] = ({ targets[i], issuer, remaining, message });

		for (sz2 = 0; sz2 < 4; sz2++) {
			int len2;

			len2 = strlen(table[i][sz2]);

			if (len2 > len[sz2]) {
				len[sz2] = len2;
			}
		}
	}

	buffer = "\033[1m" + lalign(header, len[0])
		+ "  " + lalign("Issuer", len[1])
		+ "  " + lalign("Exp", len[2])
		+ "  Message\033[0m\n";

	for (i = 0; i < sz; i++) {
		buffer +=
			lalign(table[i][0], len[0])
			+ "  " + lalign(table[i][1], len[1])
			+ "  " + lalign(table[i][2], len[2])
			+ "  " + table[i][3] + "\n";

		if (i % 5 == 4 && i + 1 != sz) {
			buffer += "\n";
		}
	}

	return buffer;
}
