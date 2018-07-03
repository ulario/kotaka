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
#include <kernel/user.h>
#include <kotaka/log.h>
#include <kotaka/paths/account.h>
#include <kotaka/paths/ansi.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit LIB_USERD;
inherit LIB_SYSTEM_USER;

mapping connections;	/* all connections */
mapping users;		/* all logged in user objects */
mapping guests;		/* all guest user objects */

static void create()
{
	::create();

	connections = ([ ]);
	users = ([ ]);
	guests = ([ ]);

	SYSTEM_USERD->set_binary_manager(1, this_object());
}

object select(string str)
{
	int has_telnet;
	int has_mudclient;
	object conn;
	object user;
	string basename;

	conn = previous_object(2);

	while (conn && conn <- LIB_USER) {
		if (conn <- "~/obj/filter/telnet") {
			has_telnet = 1;
		}
		if (conn <- "~/obj/filter/mudclient") {
			has_mudclient = 1;
		}

		conn = conn->query_conn();
	}

	if (!conn) {
		return nil;
	}

	sscanf(object_name(conn), "%s#%*d", basename);

	switch(basename) {
	case BINARY_CONN:
		if (!has_telnet) {
			return clone_object("~/obj/filter/telnet");
		}
		if (!has_mudclient) {
			return clone_object("~/obj/filter/mudclient");
		}
		return clone_object("~/obj/user");

	case TELNET_CONN:
		if (!has_mudclient) {
			return clone_object("~/obj/filter/mudclient");
		}
		return clone_object("~/obj/user");
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

private string query_siteban_message(object LIB_CONN connection)
{
	string ip;

	while (connection && connection <- LIB_USER) {
		connection = connection->query_conn();
	}

	ip = query_ip_number(connection);

	ip = BAND->query_siteban_message(ip);

	if (ip) {
		return "\033[1;37;41mSitebanned\033[0m\n\n\033[1;31m" + ip + "\033[0m\n";
	} else {
		return "\033[1;37;41mSitebanned\033[0m\n";
	}
}

string query_sitebanned_banner(object LIB_CONN connection)
{
	return query_siteban_message(connection);
}

void siteban_notify(object LIB_CONN connection)
{
	object user;

	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	user = connection->query_user();

	while (user && user <- LIB_CONN) {
		user = user->query_user();
	}

	if (!user) {
		return;
	}

	user->message(query_siteban_message(connection));
}

string query_banner(object LIB_CONN connection)
{
	return nil;
}

int query_timeout(object LIB_CONN connection)
{
	int level;
	object conn;

	conn = connection;

	while (conn && conn <- LIB_USER) {
		level++;
		conn = conn->query_conn();
	}

	if (!conn) {
		return -1;
	}

	if (BAND->check_siteban(query_ip_number(conn))) {
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
