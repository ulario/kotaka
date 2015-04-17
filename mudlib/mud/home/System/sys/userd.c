/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2000, 2007, 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <status.h>

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

private object query_manager(object base)
{
	int port;

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

/* initd hooks */

void reboot()
{
	int i, sz;

	ACCESS_CHECK(SYSTEM());

	unregister_with_klib_userd();
	register_with_klib_userd();
}

void hotboot()
{
	int i, sz;

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

	root = connection;

	while (root && root<-LIB_USER) {
		root = root->query_conn();
	}

	TLSD->set_tls_value("System", "root-connection", root);

	userd = query_manager(root);

	TLSD->set_tls_value("System", "userd", userd);

	if (!userd) {
		TLSD->set_tls_value("System", "abort-connection", 1);

		return "Internal error: no connection manager.\n";
	}

	if (BAND->check_siteban(query_ip_number(root))) {
		TLSD->set_tls_value("System", "abort-connection", 1);

		catch {
			return userd->query_sitebanned_banner(connection);
		} : {
			return "Banned";
		}
	}

	if (free_users() < 2) {
		TLSD->set_tls_value("System", "abort-connection", 1);

		/* one for the admin, and one for dumping overloads */
		catch {
			return userd->query_overload_banner(connection);
		} : {
			return "Internal error: overload banner failed.\n";
		}
	}

	if (blocked) {
		TLSD->set_tls_value("System", "abort-connection", 1);

		catch {
			return userd->query_blocked_banner(connection);
		} : {
			return "Internal error: blocked banner failed.\n";
		}
	}

	return userd->query_banner(connection);
}

int query_timeout(object LIB_CONN connection)
{
	object userd;

	ACCESS_CHECK(SYSTEM() || KERNEL());

	if (TLSD->query_tls_value("System", "abort-connection")) {
		return -1;
	};

	userd = TLSD->query_tls_value("System", "userd");

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

	if (!has_rlimits) {
		return clone_object("~/obj/filter/rlimits");
	}

	if (intercept = TLSD->query_tls_value("System", "redirect-intercept")) {
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

		filter = clone_object("~/obj/filter/rlimits");

		TLSD->set_tls_value("System", "redirect-intercept", user);

		start->system_redirect(filter, str);

		TLSD->set_tls_value("System", "redirect-intercept", nil);

		return;
	}

	start->system_redirect(user, str);
}

void check_sitebans()
{
	object *conns;
	int sz;

	ACCESS_CHECK(ACCOUNT());

	conns = KERNELD->query_connections();

	for (sz = sizeof(conns) - 1; sz >= 0; --sz) {
		if (BAND->check_siteban(query_ip_number(conns[sz]))) {
			conns[sz]->reboot();
		}
	}
}
