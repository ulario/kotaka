/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2006, 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <kernel/user.h>
#include <kotaka/log.h>
#include <kotaka/paths/account.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit LIB_USERD;
inherit LIB_SYSTEM_USER;

mapping connections;	/* all connections */
mapping users;		/* all logged in user objects */
mapping guests;		/* all guest user objects */
int splash;

static void create()
{
	::create();

	connections = ([ ]);
	users = ([ ]);
	guests = ([ ]);

	SYSTEM_USERD->set_telnet_manager(0, this_object());
	SYSTEM_USERD->set_binary_manager(1, this_object());
}

object select(string str)
{
	int level;
	object base_conn;
	object user;
	string basename;

	base_conn = previous_object(2);

	while (base_conn && base_conn <- LIB_USER) {
		level++;
		base_conn = base_conn->query_conn();
	}

	if (!base_conn) {
		return nil;
	}

	sscanf(object_name(base_conn), "%s#%*d", basename);

	switch(basename) {
	case BINARY_CONN:
		switch(level + 1) {
		case 0:
			return clone_object("~System/obj/filter/atomic");
		case 1:
			return clone_object("~System/obj/filter/rlimits");
		case 2:
			return clone_object("~/obj/filter/telnet");
		case 3:
			return clone_object("~/obj/user");
		}

	case TELNET_CONN:
		switch(level + 1) {
		case 0:
			return clone_object("~System/obj/filter/atomic");
		case 1:
			return clone_object("~System/obj/filter/rlimits");
		case 2:
			return clone_object("~/obj/user");
		}
	}
}

string query_blocked_banner(object LIB_CONN connection)
{
	return "Sorry, but due to system maintenance you cannot log in right now.\n";
}

string query_overload_banner(object LIB_CONN connection)
{
	return "There are too many connections to this mud.\n";
}

int is_sitebanned(string ip)
{
	string o1, o2, o3, o4;

	if (sscanf(ip, "%s.%s.%s.%s", o1, o2, o3, o4) < 4) {
		/* weird IP? */
		return 0;
	}

	if (BAND->query_is_site_banned(o1 + "." + o2 + "." + o3 + "." + o4)) {
		return 1;
	}

	if (BAND->query_is_site_banned(o1 + "." + o2 + "." + o3 + ".*")) {
		return 1;
	}

	if (BAND->query_is_site_banned(o1 + "." + o2 + ".*.*")) {
		return 1;
	}

	if (BAND->query_is_site_banned(o1 + ".*.*.*")) {
		return 1;
	}

	return 0;
}

string query_banner(object LIB_CONN connection)
{
	string *files;
	string ansi;
	int sz;
	int level;
	object base_conn;

	base_conn = connection;

	while (base_conn && base_conn <- LIB_USER) {
		level++;
		base_conn = base_conn->query_conn();
	}

	if (is_sitebanned(query_ip_number(base_conn))) {
		return "You are sitebanned.\n";
	}

	if (!base_conn) {
		return "";
	}

	if (level == 0) {
		int rnd;

		files = get_dir("~/data/splash/telnet_banners/chars/*")[0];
		sz = sizeof(files);

		if (splash >= sz) {
			splash = 0;
		}

		ansi = read_file("~/data/splash/telnet_banners/ansi/" + files[splash]);

		if (!ansi) {
			ansi = STRINGD->simple_ansify(
				read_file("~/data/splash/telnet_banners/chars/" + files[splash]),
				read_file("~/data/splash/telnet_banners/fgcolor/" + files[splash]),
				read_file("~/data/splash/telnet_banners/bgcolor/" + files[splash])
			);

			write_file("~/data/splash/telnet_banners/ansi/" + files[splash], ansi);
		};

		splash++;

		return ansi;
	}
}

int query_timeout(object LIB_CONN connection)
{
	int level;
	object base_conn;

	base_conn = connection;

	while (base_conn && base_conn <- LIB_USER) {
		level++;
		base_conn = base_conn->query_conn();
	}

	if (!base_conn) {
		return -1;
	}

	if (is_sitebanned(query_ip_number(base_conn))) {
		return -1;
	}

	if (level == 0) {
		connections[connection] = 1;
		connection(connection);
		::set_mode(MODE_RAW);
		redirect(select(nil), nil);
	}

	return 5;
}

void upgrade()
{
	if (!guests) {
		guests = ([ ]);
	}

	LOGD->post_message("Text", LOG_INFO, "Text user manager upgraded.");
}

void add_user(string name, object user)
{
	ACCESS_CHECK(TEXT());

	if (users[name]) {
		error("Duplicate user");
	}

	users[name] = user;
}

void delete_user(string name)
{
	ACCESS_CHECK(TEXT());

	if (!users[name]) {
		error("No such user");
	}

	users[name] = nil;
}

void add_guest(object user)
{
	if (guests[user]) {
		error("Duplicate guest");
	}

	guests[user] = 1;
}

void delete_guest(object user)
{
	if (!guests[user]) {
		error("No such guest");
	}

	guests[user] = nil;
}

int query_is_guest(object user)
{
	return !!guests[user];
}

void promote_guest(string name, object user)
{
	ACCESS_CHECK(TEXT() || GAME());

	if (!guests[user]) {
		error("No such guest");
	}

	if (users[name]) {
		error("Duplicate user");
	}

	guests[user] = nil;
	users[name] = user;
}

void rename_user(string oldname, string newname)
{
	ACCESS_CHECK(TEXT() || GAME());

	if (!users[oldname]) {
		error("No such user");
	}

	if (users[newname]) {
		error("Duplicate user");
	}

	users[newname] = users[oldname];
	users[oldname] = nil;
}

object find_user(string name)
{
	return users[name];
}

string *query_names()
{
	return map_indices(users);
}

object *query_users()
{
	return map_values(users);
}

object *query_guests()
{
	return map_indices(guests);
}

object *query_connections()
{
	return map_indices(connections);
}
