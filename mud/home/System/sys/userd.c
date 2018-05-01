/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <status.h>

#define SITEBAN_DELAY 0.05 /* linger time for dumping a sitebanned connection */
#define OVERLOAD_DELAY 1.0 /* linger time for dumping an overloaded connection */
#define BLOCK_DELAY 5.0 /* linger time for dumping a blocked connection */

inherit SECOND_AUTO;
inherit LIB_USERD;

inherit userd API_USER;
inherit user LIB_USER;

object *connections;

/*************/
/* Variables */
/*************/

mapping telnet_managers;	/* managers assigned to logical telnet ports */
mapping binary_managers;	/* managers assigned to logical binary ports */

int binary_port_count;	/* number of ports currently registered */
int telnet_port_count;	/* number of ports currently registered */
int blocked;		/* connection blocking is active */
mapping reblocked;	/* objects that were already manually blocked */

/***************/
/* Definitions */
/***************/

/* internal */

private void register_with_klib_userd();

static void create()
{
	user::create();
	userd::create();

	binary_managers = ([ ]);
	telnet_managers = ([ ]);

	reblocked = ([ ]);

	register_with_klib_userd();
}

private void register_with_klib_userd()
{
	mixed *status;
	int index;
	object this;
	this = this_object();

	status = status();

	telnet_port_count = sizeof(status(ST_TELNETPORTS));
	binary_port_count = sizeof(status(ST_BINARYPORTS));

	USERD->set_telnet_manager(0, this);

	for (index = 1; index < telnet_port_count; index++) {
		USERD->set_telnet_manager(index, this);
	}

	for (index = 1; index < binary_port_count; index++) {
		USERD->set_binary_manager(index, this);
	}
}

private void unregister_with_klib_userd()
{
	mixed *status;
	int index;

	status = status();

	for (index = 0; index < telnet_port_count; index++) {
		USERD->set_telnet_manager(index, nil);
	}

	for (index = 0; index < binary_port_count; index++) {
		USERD->set_binary_manager(index, nil);
	}

	telnet_port_count = 0;
	binary_port_count = 0;
}

static void timeout(object conn)
{
	if (conn && conn->query_user() == this_object()) {
		connection(conn);
		disconnect();
	}
}

private object conn_of(object obj)
{
	while (obj && obj<-LIB_USER) {
		obj = obj->query_conn();
	}

	if (obj<-LIB_CONN) {
		return obj;
	}
}

private object query_manager(object LIB_CONN base)
{
	int port;

	ASSERT(base);

	port = base->query_port();

	if (base <- TELNET_CONN) {
		return telnet_managers[port];
	}

	if (base <- BINARY_CONN) {
		return binary_managers[port];
	}

	if (base <- SYSTEM_CONN) {
		return base->query_manager();
	}

	error("Unrecognized connection");
}

/* external */

int free_users()
{
	return status(ST_UTABSIZE) - sizeof(userd::query_connections());
}

void block_connections(varargs object except)
{
	int index;

	ACCESS_CHECK(SYSTEM() || KADMIN() || INTERFACE());

	if (blocked) {
		return;
	}

	blocked = 1;
	reblocked = ([ ]);

	connections = userd::query_connections();
	connections -= ({ except });

	for (index = 0; index < sizeof(connections); index++) {
		object conn;
		object user;

		conn = connections[index];
		user = conn->query_user();

		/* don't interfere with the failsafe */
		if (user && user <- "/kernel/obj/user") {
			continue;
		}

		if (conn->query_mode() == MODE_BLOCK) {
			reblocked[conn] = 1;
		} else {
			conn->set_mode(MODE_BLOCK);
		}
	}
}

void unblock_connections(varargs object except)
{
	int index;

	ACCESS_CHECK(SYSTEM() || KADMIN() || INTERFACE());

	if (!blocked) {
		return;
	}

	blocked = 0;

	connections = userd::query_connections();
	connections -= ({ except });

	for (index = 0; index < sizeof(connections); index++) {
		object conn;
		conn = connections[index];

		if (reblocked[conn]) {
			reblocked[conn] = nil;
		} else {
			conn->set_mode(MODE_UNBLOCK);
		}
	}

	reblocked = nil;
	connections = nil;
}

int query_blocked()
{
	return blocked;
}

void set_binary_manager(int port, object LIB_USERD manager)
{
	ACCESS_CHECK(INTERFACE());

	PERMISSION_CHECK(port != 0);

	binary_managers[port] = manager;
}

void set_telnet_manager(int port, object LIB_USERD manager)
{
	ACCESS_CHECK(INTERFACE());

	telnet_managers[port] = manager;
}

/* initd hooks */

void reboot()
{
	ACCESS_CHECK(SYSTEM());

	unregister_with_klib_userd();
	register_with_klib_userd();
}

void hotboot()
{
	ACCESS_CHECK(SYSTEM());

	unregister_with_klib_userd();
	register_with_klib_userd();
}

/* userd hooks */

string query_banner(object LIB_CONN connection)
{
	object userd;
	object root;
	string ip;

	ACCESS_CHECK(SYSTEM() || KERNEL());

	root = conn_of(connection);

	ASSERT(root);

	userd = query_manager(root);

	if (!userd) {
		TLSD->set_tls_value("System", "abort-connection", 1);

		return "No connection manager\n";
	}

	if (free_users() == 0) {
		/* if we're full, close it immediately to avoid a DoS against the admin port */
		TLSD->set_tls_value("System", "abort-connection", 1);
		TLSD->set_tls_value("System", "abort-delay", -1);

		root->set_mode(MODE_BLOCK);

		catch {
			return userd->query_overload_banner(connection);
		} : {
			return "Connection manager fault\n\nSystem busy\n";
		}
	}

	if (BAND->check_siteban(query_ip_number(root))) {
		/* if the IP is sitebanned send it away */
		TLSD->set_tls_value("System", "abort-connection", 1);
		TLSD->set_tls_value("System", "abort-delay", free_users() > 5 ? SITEBAN_DELAY : 0);

		root->set_mode(MODE_BLOCK);

		catch {
			return userd->query_sitebanned_banner(connection);
		} : {
			/* if they're sitebanned we don't need to give any details */
			return "Sitebanned\n";
		}
	}

	if (free_users() + 1 < 2) {
		/* we're too full, close the connection */
		TLSD->set_tls_value("System", "abort-connection", 1);
		TLSD->set_tls_value("System", "abort-delay", OVERLOAD_DELAY);

		root->set_mode(MODE_BLOCK);

		catch {
			return userd->query_overload_banner(connection);
		} : {
			return "Connection manager fault\n\nSystem busy\n";
		}
	}

	if (blocked) {
		/* check this after overloads.  overloads are more serious */
		TLSD->set_tls_value("System", "abort-connection", 1);
		TLSD->set_tls_value("System", "abort-delay", BLOCK_DELAY);

		root->set_mode(MODE_BLOCK);

		catch {
			return userd->query_blocked_banner(connection);
		} : {
			return "Connection manager fault\n\nSystem suspended\n";
		}
	}

	return userd->query_banner(connection);
}

int query_timeout(object LIB_CONN connection)
{
	object userd;
	object root;

	ACCESS_CHECK(SYSTEM() || KERNEL());

	if (TLSD->query_tls_value("System", "abort-connection")) {
		mixed delay;

		delay = TLSD->query_tls_value("System", "abort-delay");

		if (!delay || (float)delay < 0.0) {
			return -1;
		}

		connection->set_mode(MODE_BLOCK);

		SUSPENDD->queue_delayed_work("close_connection", delay, connection);

		return (int)(ceil((float)delay));
	};

	root = conn_of(connection);

	ASSERT(root);

	userd = query_manager(root);

	if (!userd) {
		return -1;
	}

	return userd->query_timeout(connection);
}

object select(string str)
{
	object userd;
	object conn;
	object user;
	object intercept;

	int has_rlimits;

	ACCESS_CHECK(SYSTEM() || KERNEL());

	conn = previous_object(1);

	while (conn && conn <- LIB_USER) {
		if (conn <- "~/obj/filter/rlimits") {
			has_rlimits = 1;
		}

		conn = conn->query_conn();
	}

	userd = query_manager(conn);

	if (!userd) {
		TLSD->set_tls_value("System", "select-intercept", nil);
		TLSD->set_tls_value("System", "userd-error", "No connection manager");

		return this_object();
	}

	if (intercept = TLSD->query_tls_value("System", "select-intercept")) {
		TLSD->set_tls_value("System", "select-intercept", nil);

		return intercept;
	}

	user = userd->select(str);

	if (!user) {
		TLSD->set_tls_value("System", "userd-error", "Connection manager returned nil");

		return this_object();
	}

	if (has_rlimits) {
		return user;
	} else {
		TLSD->set_tls_value("System", "select-intercept", user);

		return clone_object("~/obj/filter/rlimits", user->query_owner());
	}
}

/* connection hooks */

private void handle_message(object conn, string str)
{
	string errmsg;

	errmsg = TLSD->query_tls_value("System", "userd-error");

	previous_object()->message(errmsg ? errmsg  + "\r\n" : "Unknown connection manager error\r\n");
}

int receive_message(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	handle_message(previous_object(), str);

	return MODE_DISCONNECT;
}

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	handle_message(previous_object(), str);

	return MODE_DISCONNECT;
}

/* interception */
void intercept_redirect(object user, string str)
{
	object start;
	object conn;
	int has_rlimits;

	ACCESS_CHECK(SYSTEM());

	conn = previous_object();
	start = conn;

	while (conn && conn <- LIB_USER) {
		if (conn <- "~/obj/filter/rlimits") {
			has_rlimits = 1;
		}

		conn = conn->query_conn();
	}

	if (!has_rlimits) {
		object filter;

		filter = clone_object("~/obj/filter/rlimits", user->query_owner());

		TLSD->set_tls_value("System", "select-intercept", user);

		start->system_redirect(filter, str);

		TLSD->set_tls_value("System", "select-intercept", nil);

		return;
	}

	start->system_redirect(user, str);
}

void check_sitebans()
{
	object *conns;
	int sz;
	int ticks;

	ACCESS_CHECK(ACCOUNT());

	conns = KERNELD->query_connections();
	ticks = status(ST_TICKS);

	rlimits(0; -1) {
		for (sz = sizeof(conns); --sz >= 0; ) {
			object conn;

			conn = conns[sz];

			if (BAND->check_siteban(query_ip_number(conn))) {
				object manager;

				manager = query_manager(conn);

				if (manager && function_object("siteban_notify", manager)) {
					catch {
						rlimits(0; ticks) {
							manager->siteban_notify(conn);
						}
					}
				}

				conn->reboot();
			}
		}
	}
}

void close_connection(object conn)
{
	ACCESS_CHECK(SYSTEM());

	if (conn) {
		conn->reboot();
	}
}
