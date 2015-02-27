/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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

/* keeps track of bans */

mapping bans;
mapping sitebans;

private void save();
private void restore();

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

void ban_site(string site)
{
	ACCESS_CHECK(INTERFACE() || GAME());

	if (site == "127.0.0.1" || site == "::1") {
		error("Cannot ban localhost");
	}

	if (sitebans[site]) {
		error("Site already banned");
	}

	sitebans[site] = 1;
	save();
}

void unban_site(string site)
{
	ACCESS_CHECK(INTERFACE() || KADMIN() || GAME());

	if (!sitebans[site]) {
		error("Site not banned");
	}

	sitebans[site] = nil;
	save();
}

void ban_username(string username)
{
	ACCESS_CHECK(INTERFACE() || GAME());

	if (username == "admin") {
		error("Cannot ban admin");
	}

	if (bans[username]) {
		error("Username already banned");
	}

	bans[username] = 1;
	save();
}

void unban_username(string username)
{
	ACCESS_CHECK(INTERFACE() || GAME());

	if (!bans[username]) {
		error("Username not banned");
	}

	bans[username] = nil;
	save();
}

int query_is_username_banned(string username)
{
	return !!bans[username];
}

int query_is_site_banned(string site)
{
	return !!sitebans[site];
}

string *query_username_bans()
{
	return map_indices(bans);
}

string *query_site_bans()
{
	return map_indices(sitebans);
}

void force_save()
{
	ACCESS_CHECK(INTERFACE() || GAME() || KADMIN() || ACCOUNT());

	save();
}

void force_restore()
{
	ACCESS_CHECK(INTERFACE() || GAME() || KADMIN() || ACCOUNT());

	restore();
}

private void save()
{
	string buf;

	buf = STRINGD->hybrid_sprint( ([ "bans": bans, "sitebans": sitebans ]) );

	SECRETD->make_dir(".");
	SECRETD->remove_file("bans-tmp");
	SECRETD->write_file("bans-tmp", buf + "\n");
	SECRETD->remove_file("bans");
	SECRETD->rename_file("bans-tmp", "bans");
}

private void restore()
{
	string buf;

	buf = SECRETD->read_file("bans");

	if (buf) {
		mapping save;

		save = PARSER_VALUE->parse(buf);

		bans = save["bans"];
		sitebans = save["sitebans"];
	}
}
