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
inherit "/lib/sort";
inherit "~System/lib/string/align";

string *query_parse_methods()
{
	return ({ "raw" });
}

int compare_sites(string a, string b)
{
	int o1a, o2a, o3a, o4a, cidra;
	int o1b, o2b, o3b, o4b, cidrb;

	sscanf(a, "%d.%d.%d.%d/%d", o1a, o2a, o3a, o4a, cidra);
	sscanf(b, "%d.%d.%d.%d/%d", o1b, o2b, o3b, o4b, cidrb);

	if (o1a < o1b) {
		return -1;
	}
	if (o1a > o1b) {
		return 1;
	}
	if (o2a < o2b) {
		return -1;
	}
	if (o2a > o2b) {
		return 1;
	}
	if (o3a < o3b) {
		return -1;
	}
	if (o3a > o3b) {
		return 1;
	}
	if (o4a < o4b) {
		return -1;
	}
	if (o4a > o4b) {
		return 1;
	}
	if (cidra < cidrb) {
		return -1;
	}
	if (cidra > cidrb) {
		return 1;
	}
	return 0;
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

	qsort(sites, 0, sz, "compare_sites");

	if (sz) {
		mapping *bans;
		int i;

		bans = allocate(sz);

		for (i = 0; i < sz; i++) {
			bans[i] = BAND->query_siteban(sites[i]);
		}

		send_out("bans"->print_bans("Site", sites, bans));

		send_out("\n");
	} else {
		send_out("There are no banned sites.\n");
	}
}
