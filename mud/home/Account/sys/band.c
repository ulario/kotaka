/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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

void ban_site(string site, varargs string message)
{
	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	if (sscanf(site, "127.%*s")) {
		error("Cannot ban localhost");
	}

	if (site == "::1") {
		error("Cannot ban localhost");
	}

	sitebans[site] = message ? message : 1;
	save();
	SYSTEM_USERD->check_sitebans();
}

void unban_site(string site)
{
	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	if (!sitebans[site]) {
		error("Site not banned");
	}

	sitebans[site] = nil;
	save();
}

void ban_user(string username, varargs string message)
{
	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	if (username == "admin") {
		error("Cannot ban admin");
	}

	bans[username] = message ? message : 1;
	save();
}

void unban_user(string username)
{
	ACCESS_CHECK(GAME() || INTERFACE() || KADMIN());

	if (!bans[username]) {
		error("Username not banned");
	}

	bans[username] = nil;
	save();
}

int query_is_user_banned(string username)
{
	return !!bans[username];
}

string query_ban_message(string username)
{
	mixed message;

	message = bans[username];

	if (typeof(message) == T_STRING) {
		return message;
	} else {
		return nil;
	}
}

int query_is_site_banned(string site)
{
	return !!sitebans[site];
}

string query_siteban_message(string site)
{
	mixed message;

	message = sitebans[site];

	if (typeof(message) == T_STRING) {
		return message;
	} else {
		return nil;
	}
}

string *query_bans()
{
	return map_indices(bans);
}

string *query_sitebans()
{
	return map_indices(sitebans);
}

void save()
{
	string buf;

	ACCESS_CHECK(ACCOUNT() || GAME() || INTERFACE() || KADMIN() || VERB());

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
}

int check_siteban(string ip)
{
	string o1, o2, o3, o4;

	if (sscanf(ip, "%s.%s.%s.%s", o1, o2, o3, o4) < 4) {
		/* weird IP? */
		return 0;
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
