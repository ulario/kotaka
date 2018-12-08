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
#include <kotaka/assert.h>
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

static void destruct()
{
	object *turkeys;
	int sz;

	turkeys = map_values(users) + map_indices(guests);

	for (sz = sizeof(turkeys); --sz >= 0; ) {
		catch {
			destruct_object(turkeys[sz]);
		}
	}
}

string query_blocked_banner(object LIB_CONN connection)
{
	return "Maintenance in progress\n";
}

string query_overloaded_banner(object LIB_CONN connection)
{
	return "Too many connections\n";
}

string query_sitebanned_banner(object LIB_CONN connection)
{
	string ip;
	mapping ban;
	string output;

	while (connection && connection <- LIB_USER) {
		connection = connection->query_conn();
	}

	ip = query_ip_number(connection);

	ASSERT(ip);

	LOGD->post_message("system", LOG_NOTICE, "Telnet connection from banned ip " + ip);

	ban = BAND->check_siteban(ip);

	ASSERT(ban);

	output = "You are sitebanned\n";

	if (ban) {
		string message;
		mixed expire;

		if (message = ban["message"]) {
			output += message + "\n";
		}

		expire = ban["expire"];

		if (expire != nil) {
			int remaining;

			remaining = expire - time();

			if (remaining < 60) {
				output += "(expires in " + remaining + " seconds)\n";
			} else if (remaining < 3600) {
				output += "(expires in " + (remaining / 60 + 1) + " minutes)\n";
			} else if (remaining < 86400) {
				output += "(expires in " + (remaining / 3600 + 1) + " hours)\n";
			} else {
				output += "(expires in " + (remaining / 86400 + 1) + " days)\n";
			}
		}
	}

	return output;
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

	user->quit("sitebanned");
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

object select(string str)
{
	int has_telnet, has_mudclient;
	object conn;
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
		/* fall through */

	case TELNET_CONN:
		if (!has_mudclient) {
			return clone_object("~/obj/filter/mudclient");
		}
		return clone_object("~/obj/user");
	}
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

void demote_user(string name)
{
	object user;

	ACCESS_CHECK(TEXT() || GAME());

	user = users[name];

	if (!user) {
		error("No such user");
	}

	ASSERT(!guests[user]);

	guests[user] = 1;
	users[name] = nil;
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
