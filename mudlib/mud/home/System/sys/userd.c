/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2007, 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit SECOND_AUTO;

inherit userd API_USER;
inherit user LIB_USER;

int reserve;
object *connections;
object intercept;

/*************/
/* Variables */
/*************/

mapping telnet_managers;	/* managers assigned to logical telnet ports */
mapping binary_managers;	/* managers assigned to logical binary ports */

int enabled;		/* active or not */
int binary_port_count;	/* number of ports currently registered */
int telnet_port_count;	/* number of ports currently registered */
int blocked;		/* connection blocking is active */
mapping reblocked;	/* objects that were already manually blocked */

/***************/
/* Definitions */
/***************/

/* internal */

static void create()
{
	user::create();
	userd::create();

	binary_managers = ([ ]);
	telnet_managers = ([ ]);

	reblocked = ([ ]);
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

	for (index = 0; index < telnet_port_count; index++) {
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

private object query_manager(object conn)
{
	object base;
	int port;

	base = conn;

	while (base <- LIB_USER) {
		base = base->query_conn();
	}

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

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	register_with_klib_userd();
	enabled = 1;
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	enabled = 0;
	unregister_with_klib_userd();
}

void block_connections(varargs object except)
{
	int index;

	ACCESS_CHECK(SYSTEM() || KADMIN() || INTERFACE());

	if (blocked) {
		blocked++;
		return;
	}

	LOGD->post_message("userd", LOG_NOTICE, "Blocking connections");

	blocked = 1;
	reblocked = ([ ]);

	connections = userd::query_connections();
	connections -= ({ except });

	for (index = 0; index < sizeof(connections); index++) {
		object conn;

		conn = connections[index];

		/* don't interfere with the failsafe */
		if (conn->query_user() <- "/kernel/obj/user") {
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

	if (blocked == 0) {
		error("Connections not blocked");
	}

	if (blocked > 1) {
		blocked--;
		return;
	}

	LOGD->post_message("userd", LOG_NOTICE, "Unblocking connections");

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

void set_reserve(int new_reserve)
{
	ACCESS_CHECK(PRIVILEGED());

	reserve = new_reserve;
}

int query_reserve()
{
	return reserve;
}

/* initd hooks */

void reboot()
{
	int i, sz;

	ACCESS_CHECK(SYSTEM());

	if (enabled) {
		unregister_with_klib_userd();
		register_with_klib_userd();
	}
}

/* userd hooks */

string query_banner(object LIB_CONN connection)
{
	object userd;

	ACCESS_CHECK(SYSTEM() || KERNEL());

	userd = query_manager(connection);

	if (!userd) {
		return "Internal error: no connection manager\n";
	}

	if (free_users() < reserve) {
		return userd->query_overload_banner(connection);
	}

	if (blocked) {
		return userd->query_blocked_banner(connection);
	}

	return userd->query_banner(connection);
}

int query_timeout(object LIB_CONN connection)
{
	object userd;

	ACCESS_CHECK(SYSTEM() || KERNEL());

	userd = query_manager(connection);

	if (!userd || blocked || free_users() < reserve) {
		return -1;
	}

	return userd->query_timeout(connection);
}

object select(string str)
{
	object userd;
	object conn;
	object user;

	int has_rlimits;

	ACCESS_CHECK(SYSTEM() || KERNEL());

	conn = previous_object(1);

	while (conn && conn <- LIB_USER) {
		if (conn <- "~/obj/filter/rlimits") {
			has_rlimits = 1;
		}

		conn = conn->query_conn();
	}

	if (!has_rlimits) {
		return clone_object("~/obj/filter/rlimits");
	}

	if (intercept) {
		user = intercept;

		intercept = nil;

		return user;
	}

	userd = query_manager(conn);

	if (!userd) {
		return this_object();
	}

	return userd->select(str);
}

/* connection hooks */

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	previous_object()->message("Internal error: connection manager fault\n");

	return MODE_DISCONNECT;
}

int receive_message(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	previous_object()->message("Internal error: connection manager fault\n");

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

		intercept = user;

		filter = clone_object("~/obj/filter/rlimits");

		start->system_redirect(filter, str);

		intercept = nil;

		return;
	}

	start->system_redirect(user, str);
}
