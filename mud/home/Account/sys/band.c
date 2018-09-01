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
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <type.h>

/* keeps track of bans */

mapping bans;
mapping sitebans;

void save();
void restore();
static void prune_bans();
static void prune_sitebans();

static void create()
{
	restore();

	if (!bans) {
		bans = ([ ]);
	}

	if (!sitebans) {
		sitebans = ([ ]);
	}
}

void save()
{
	string buf;

	ACCESS_CHECK(KERNEL() || ACCOUNT() || GAME() || INTERFACE() || KADMIN() || VERB());

	buf = STRINGD->hybrid_sprint( ([ "bans": bans, "sitebans": sitebans ]) );

	CONFIGD->make_dir(".");
	CONFIGD->remove_file("bans-tmp");
	CONFIGD->write_file("bans-tmp", buf + "\n");
	CONFIGD->remove_file("bans");
	CONFIGD->rename_file("bans-tmp", "bans");
}

void restore()
{
	string buf;

	ACCESS_CHECK(ACCOUNT() || GAME() || INTERFACE() || KADMIN() || VERB());

	buf = CONFIGD->read_file("bans");

	if (buf) {
		mapping save;

		save = PARSER_VALUE->parse(buf);

		bans = save["bans"];
		sitebans = save["sitebans"];
	}

	call_out("prune_bans", 0);
	call_out("prune_sitebans", 0);
}

static void prune_bans()
{
	string *indices;
	int sz;

	indices = map_indices(bans);

	for (sz = sizeof(indices); --sz >= 0; ) {
		mixed ban;
		string username;

		username = indices[sz];

		ban = bans[username];

		switch(typeof(ban)) {
		case T_NIL: /* no ban */
		case T_INT: /* blank ban */
		case T_STRING: /* ban with message */
			continue;
		case T_ARRAY: /* modern ban */
			{
				string message;
				int expiry;

				({ message, expiry }) = ban;

				if (expiry != -1) {
					if (time() >= expiry) {
						/* expired */
						bans[username] = nil;
						call_out("save", 0);
					}
				}
			}
		}
	}
}

static void prune_sitebans()
{
	string *indices;
	int sz;

	indices = map_indices(sitebans);

	for (sz = sizeof(indices); --sz >= 0; ) {
		mixed ban;
		string site;

		username = indices[sz];
		ban = sitebans[site];

		switch(typeof(ban)) {
		case T_NIL: /* no ban */
		case T_INT: /* blank ban */
		case T_STRING: /* ban with message */
			continue;
		case T_ARRAY: /* modern ban */
			{
				string message;
				int expiry;

				({ message, expiry }) = ban;

				if (expiry != -1) {
					if (time() >= expiry) {
						/* expired */
						sitebans[site] = nil;
						call_out("save", 0);
					}
				}
			}
		}
	}
}

/* API */

void ban_user(string username, string message, int expiry)
{
	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	if (username == "admin") {
		error("Cannot ban admin");
	}

	bans[username] = ({ message, expiry });
	call_out("save", 0);
}

void unban_user(string username)
{
	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	bans[username] = nil;
	call_out("save", 0);
}

int query_is_user_banned(string username)
{
	mixed ban;

	ban = bans[username];

	switch(typeof(ban)) {
	case T_NIL: /* no ban */
		return 0;
	case T_INT: /* blank ban */
		return !!ban;
	case T_STRING: /* ban with message */
		return 1;
	case T_ARRAY: /* modern ban */
		{
			string message;
			int expiry;

			({ message, expiry }) = ban;

			if (expiry != -1) {
				if (time() >= expiry) {
					/* expired */
					bans[username] = nil;
					call_out("save", 0);
					return 0;
				}
			}

			return 1;
		}
	}
}

string query_ban_message(string username)
{
	mixed ban;

	ban = bans[username];

	switch(typeof(ban)) {
	case T_NIL: /* no ban */
		return nil;
	case T_INT: /* blank ban */
		return nil;
	case T_STRING: /* ban with message */
		return ban;
	case T_ARRAY: /* modern ban */
		{
			string message;
			int expiry;

			({ message, expiry }) = ban;

			if (expiry != -1) {
				if (time() >= expiry) {
					/* expired */
					bans[username] = nil;
					call_out("save", 0);
					return nil;
				}
			}

			return message;
		}
	}
}

string *query_bans()
{
	prune_bans();

	return map_indices(bans);
}

void ban_site(string site, string message, int expiry)
{
	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	if (sscanf(site, "127.%*s")) {
		error("Cannot ban localhost");
	}

	if (site == "::1") {
		error("Cannot ban localhost");
	}

	sitebans[site] = ({ message, expiry });
	SYSTEM_USERD->check_sitebans();
	call_out("save", 0);
}

void unban_site(string site)
{
	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	sitebans[site] = nil;
	call_out("save", 0);
}

int query_is_site_banned(string site)
{
	mixed ban;

	ban = sitebans[site];

	switch(typeof(ban)) {
	case T_NIL: /* no ban */
		return 0;
	case T_INT: /* blank ban */
		return !!ban;
	case T_STRING: /* ban with message */
		return 1;
	case T_ARRAY: /* modern ban */
		{
			string message;
			int expiry;

			({ message, expiry }) = ban;

			if (expiry != -1) {
				if (time() >= expiry) {
					/* expired */
					bans[site] = nil;
					call_out("save", 0);
					return 0;
				}
			}

			return 1;
		}
	}
}

string query_siteban_message(string site)
{
	mixed ban;

	ban = sitebans[site];

	switch(typeof(ban)) {
	case T_NIL: /* no ban */
		return nil;
	case T_INT: /* blank ban */
		return nil;
	case T_STRING: /* ban with message */
		return ban;
	case T_ARRAY: /* modern ban */
		{
			string message;
			int expiry;

			({ message, expiry }) = ban;

			if (expiry != -1) {
				if (time() >= expiry) {
					/* expired */
					bans[site] = nil;
					call_out("save", 0);
					return nil;
				}
			}

			return message;
		}
	}
}

string *query_sitebans()
{
	prune_sitebans();

	return map_indices(sitebans);
}

int check_siteban(string ip)
{
	int o1, o2, o3, o4;

	if (sscanf(ip, "%d.%d.%d.%d", o1, o2, o3, o4) < 4) {
		error("Malformed IP address");
	}

	if (query_is_site_banned(o1 + "." + o2 + "." + o3 + "." + o4)) {
		return 1;
	}

	if (query_is_site_banned(o1 + "." + o2 + "." + o3 + ".*")) {
		return 1;
	}

	if (query_is_site_banned(o1 + "." + o2 + ".*.*")) {
		return 1;
	}

	if (query_is_site_banned(o1 + ".*.*.*")) {
		return 1;
	}

	return 0;
}

string check_siteban_message(string ip)
{
	int o1, o2, o3, o4;

	if (sscanf(ip, "%d.%d.%d.%d", o1, o2, o3, o4) < 4) {
		error("Malformed IP address");
	}

	ip = o1 + "." + o2 + "." + o3 + "." + o4;

	if (query_is_site_banned(ip)) {
		query_siteban_message(ip);
	}

	ip = o1 + "." + o2 + "." + o3 + ".*";

	if (query_is_site_banned(ip)) {
		query_siteban_message(ip);
	}

	ip = o1 + "." + o2 + ".*.*";

	if (query_is_site_banned(ip)) {
		query_siteban_message(ip);
	}

	ip = o1 + ".*.*.*";

	if (query_is_site_banned(ip)) {
		query_siteban_message(ip);
	}
}
