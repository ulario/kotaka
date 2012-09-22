/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>

inherit LIB_USERD;
inherit LIB_SYSTEM_USER;

mapping connections;
mapping users;
mapping guests;
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
		if (previous_object(2) <- "~Game/obj/filter/telnet") {
			user = clone_object("~Game/obj/user");
			return user;
		} else {
			return clone_object("~Game/obj/filter/telnet");
		}
	case TELNET_CONN:
		user = clone_object("~Game/obj/user");
		return user;
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

	if (!base_conn) {
		return "";
	}

	if (level == 1) {
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

	if (level == 1) {
		connections[connection] = 1;
		connection(connection);
		::set_mode(MODE_RAW);
		redirect(select(nil), nil);
	}

	return 1;
}

void upgrade()
{
	if (!guests) {
		guests = ([ ]);
	}

	LOGD->post_message("game", LOG_INFO, "Game user manager upgraded.");
}

void add_user(string name, object user)
{
	ACCESS_CHECK(GAME());

	if (users[name]) {
		error("Duplicate user");
	}

	users[name] = user;
}

void delete_user(string name)
{
	ACCESS_CHECK(GAME());

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
	ACCESS_CHECK(GAME());

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
	ACCESS_CHECK(GAME());

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
	ACCESS_CHECK(GAME());

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
	ACCESS_CHECK(PRIVILEGED());

	return map_indices(connections);
}
