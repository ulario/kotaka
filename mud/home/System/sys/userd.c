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
#include <kotaka/paths/account.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>

#define SITEBAN_DELAY  0.5 /* linger time for dumping a sitebanned connection */
#define OVERLOAD_DELAY 0.5 /* linger time for dumping an overloaded connection */
#define BLOCK_DELAY    0.5 /* linger time for dumping a blocked connection */

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

int blocked;			/* connection blocking is active */
mapping reblocked;		/* objects that were already manually blocked */

/***************/
/* Definitions */
/***************/

/* internal */

void enable();

static void create()
{
	user::create();
	userd::create();

	binary_managers = ([ ]);
	telnet_managers = ([ ]);

	reblocked = ([ ]);

	load_object(TLSD);
}

void enable()
{
	int sz;
	object this;

	ACCESS_CHECK(SYSTEM());

	this = this_object();

	for (sz = sizeof(status(ST_TELNETPORTS)); --sz >= 0; ) {
		USERD->set_telnet_manager(sz, this);
	}

	for (sz = sizeof(status(ST_BINARYPORTS)); --sz >= 1; ) {
		USERD->set_binary_manager(sz, this);
	}
}

void disable()
{
	int sz;

	ACCESS_CHECK(SYSTEM());

	for (sz = status(ST_ARRAYSIZE); --sz >= 0; ) {
		USERD->set_telnet_manager(sz, nil);
		USERD->set_binary_manager(sz, nil);
	}
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

	disable();
	enable();
}

void hotboot()
{
	ACCESS_CHECK(SYSTEM());

	disable();
	enable();
}

/* userd hooks */

string query_banner(object LIB_CONN connection)
{
	object manager;
	object root;
	string ip;

	ACCESS_CHECK(SYSTEM() || KERNEL());

	root = conn_of(connection);

	ASSERT(root);

	manager = query_manager(root);

	if (!manager) {
		TLSD->set_tls_value("System", "abort-connection", 1);

		return "No connection manager\n";
	}

	if (BAND->check_siteban(query_ip_number(root))) {
		TLSD->set_tls_value("System", "abort-connection", 1);
		TLSD->set_tls_value("System", "abort-delay", free_users() ? SITEBAN_DELAY : 0);

		root->set_mode(MODE_BLOCK);

		catch {
			return manager->query_sitebanned_banner(connection);
		} : {
			return "Sitebanned\n";
		}
	}

	if (blocked) {
		TLSD->set_tls_value("System", "abort-connection", 1);
		TLSD->set_tls_value("System", "abort-delay", free_users() ? BLOCK_DELAY : 0);

		root->set_mode(MODE_BLOCK);

		catch {
			return manager->query_blocked_banner(connection);
		} : {
			return "Connection manager fault\n\nSystem suspended\n";
		}
	}

	if (!free_users()) {
		/* current connection is occupying the last slot */
		TLSD->set_tls_value("System", "abort-connection", 1);
		TLSD->set_tls_value("System", "abort-delay", 0);

		root->set_mode(MODE_BLOCK);

		catch {
			return manager->query_overload_banner(connection);
		} : {
			return "Connection manager fault\n\nSystem busy\n";
		}
	}

	return manager->query_banner(connection);
}

int query_timeout(object LIB_CONN connection)
{
	object manager;
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

	manager = query_manager(root);

	if (!manager) {
		return -1;
	}

	return manager->query_timeout(connection);
}

object select(string str)
{
	object manager, user, conn, root;

	object intercept;

	int has_rlimits;
	int has_task;

	object target;

	/* input was received before the connection was assigned to a user object */

	ACCESS_CHECK(KERNEL());

	conn = previous_object(1);

	while (conn <- LIB_USER) {
		if (conn <- "~/obj/filter/rlimits") {
			has_rlimits = 1;
		}

		if (conn <- "~/obj/filter/task") {
			has_task = 1;
		}

		conn = conn->query_conn();
	}

	intercept = TLSD->query_tls_value("System", "select-intercept");

	if (intercept) {
		if (!has_task) {
			return clone_object("~/obj/filter/task");
		}

		if (!has_rlimits) {
			return clone_object("~/obj/filter/rlimits");
		}

		TLSD->set_tls_value("System", "select-intercept", nil);

		return intercept;
	} else {
		root = conn;

		manager = query_manager(root);

		if (!manager) {
			TLSD->set_tls_value("System", "userd-error", "No connection manager");

			return this_object();
		}

		user = manager->select(str);

		if (!user) {
			TLSD->set_tls_value("System", "userd-error", "Connection manager returned nil");

			return this_object();
		}

		if (!has_rlimits || !has_task) {
			TLSD->set_tls_value("System", "select-intercept", user);

			return clone_object("~/obj/filter/task");
		}

		return user;
	}
}

void intercept_redirect(object new_user, string str)
{
	int has_task;
	int has_rlimits;

	object conn;
	object user;

	ACCESS_CHECK(SYSTEM());

	conn = previous_object();
	user = conn;

	while (conn <- LIB_USER) {
		if (conn <- "~/obj/filter/rlimits") {
			has_rlimits = 1;
		}

		if (conn <- "~/obj/filter/task") {
			has_task = 1;
		}

		conn = conn->query_conn();
	}

	if (!has_task) {
		TLSD->set_tls_value("System", "select-intercept", new_user);

		new_user = clone_object("~/obj/filter/task");
	} else if (!has_rlimits) {
		TLSD->set_tls_value("System", "select-intercept", new_user);

		new_user = clone_object("~/obj/filter/rlimits");
	}

	user->_F_sys_redirect(new_user, str);
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

object query_this_user()
{
	return TLSD->query_tls_value("System", "this-user");
}

void set_this_user(object LIB_USER user)
{
	ACCESS_CHECK(INTERFACE());

	TLSD->set_tls_value("System", "this-user", user);
}
