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
/*

Port manager

Handles incoming connections.

*/
#include <kernel/kernel.h>
#include <kernel/user.h>

#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>

#include <status.h>

inherit SECOND_AUTO;

inherit userd API_USER;
inherit user LIB_USER;

int reserve;

/*************/
/* Variables */
/*************/

mapping telnet_managers;	/* managers assigned to logical telnet ports */
mapping binary_managers;	/* managers assigned to logical binary ports */
mapping fixed_managers;		/* managers assigned to physical ports */

int enabled;		/* active or not */
int binary_port_count;	/* number of ports currently registered */
int telnet_port_count;	/* number of ports currently registered */
int blocked;		/* connection blocking is active */
mapping reblocked;	/* objects that were already manually blocked */
int stacking;		/* if we are currently building the connection stack */

/****************/
/* Declarations */
/****************/

/* internal */

static void create();
int login(string str);
private void register_with_klib_userd();
private void unregister_with_klib_userd();
private object get_manager(object conn);

/* external */

void reboot();
void clear_ports();

void set_binary_manager(int port, object LIB_USERD manager);
void set_telnet_manager(int port, object LIB_USERD manager);
void set_fixed_manager(int port, object LIB_USERD manager);

int free_users();
void block_connections();
void unblock_connections();
int query_blocked();
int query_connection_count();
int query_reserve();
void set_reserve(int new_reserve);

/* initd hooks */

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
	fixed_managers = ([ ]);

	reblocked = ([ ]);
}

private void register_with_klib_userd()
{
	mixed *status;
	int index;
	object this;
	this = this_object();

	status = status();

	telnet_port_count = sizeof(status[ST_TELNETPORTS]);
	binary_port_count = sizeof(status[ST_BINARYPORTS]);

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

private object get_manager(object conn)
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
}

/* external */

int free_users()
{
	return status()[ST_UTABSIZE] - sizeof(userd::query_connections());
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

void block_connections()
{
	object *conns;
	int index;

	ACCESS_CHECK(SYSTEM() || KADMIN());

	if (blocked) {
		blocked++;
		return;
	}

	LOGD->post_message("userd", LOG_NOTICE, "Blocking connections");

	blocked = 1;
	reblocked = ([ ]);

	conns = userd::query_connections();

	for (index = 0; index < sizeof(conns); index++) {
		object conn;

		conn = conns[index];

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

void unblock_connections()
{
	object *conns;
	int index;

	ACCESS_CHECK(SYSTEM() || KADMIN());

	if (blocked == 0) {
		error("Connections not blocked");
	}

	if (blocked > 1) {
		blocked--;
		return;
	}

	LOGD->post_message("userd", LOG_NOTICE, "Unblocking connections");

	blocked = 0;
	conns = userd::query_connections();

	for (index = 0; index < sizeof(conns); index++) {
		object conn;
		conn = conns[index];

		if (reblocked[conn]) {
			reblocked[conn] = nil;
		} else {
			conn->set_mode(MODE_UNBLOCK);
		}
	}

	reblocked = nil;
}

int query_blocked()
{
	return blocked;
}

void reboot()
{
	if (enabled) {
		unregister_with_klib_userd();
		register_with_klib_userd();
	}
}

void set_binary_manager(int port, object LIB_USERD manager)
{
	ACCESS_CHECK(PRIVILEGED());

	PERMISSION_CHECK(port != 0);

	binary_managers[port] = manager;
}

void set_telnet_manager(int port, object LIB_USERD manager)
{
	ACCESS_CHECK(PRIVILEGED());

	telnet_managers[port] = manager;
}

void set_fixed_manager(int port, object LIB_USERD manager)
{
	ACCESS_CHECK(PRIVILEGED());

	fixed_managers[port] = manager;
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

void prepare_reboot()
{
	ACCESS_CHECK(SYSTEM());

	unregister_with_klib_userd();
}

void bogus_reboot()
{
	ACCESS_CHECK(SYSTEM());

	register_with_klib_userd();
}

/* userd hooks */

string query_banner(object LIB_CONN connection)
{
	object userd;

	ACCESS_CHECK(previous_program() == USERD || SYSTEM());

	if (stacking || !(connection <- LIB_USER)) {
		/* either busy stacking or this is the first level, ignore */
		return nil;
	}

	userd = get_manager(connection);

	if (!userd) {
		return "Internal error: no connection manager";
	}

	if (blocked) {
		return userd->query_blocked_banner(connection);
	} else if (free_users() < reserve) {
		return userd->query_overload_banner(connection);
	} else {
		/* stack built and we're not naked */
		/* it is safe to call the lieutenant */
		return userd->query_banner(connection);
	}
}

int query_timeout(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == USERD || SYSTEM());

	if (stacking) {
		/* stacking in progress */
		return 0;
	} else if (!(connection <- LIB_USER)) {
		/* first level, ignore */
		stacking = 1;
		connection(connection);
		redirect(clone_object("~/obj/filter/rlimits"), nil);
		return 0;
	}

	if (blocked || free_users() < reserve) {
		return -1;
	} else {
		/* stack built and we're not naked */
		/* it is safe to call the lieutenant */
		object userd;

		userd = get_manager(connection);

		if (!userd) {
			return -1;
		}

		return userd->query_timeout(connection);
	}
}

object select(string str)
{
	object connection;
	/* do not call to lieutenant until after */
	/* connection stack is built */

	ACCESS_CHECK(previous_program() == USERD || SYSTEM());
	connection = previous_object(1);

	if (stacking) {
		/* stacking in progress */
		ASSERT(str == nil);
		return this_object();
	} else if (!(connection <- LIB_USER)) {
		/* first level, ignore */
		return this_object();
	} else {
		/* stack built and we're not naked */
		/* it is safe to call the lieutenant */
		object userd;

		userd = get_manager(connection);

		if (!userd) {
			return this_object();
		}

		return userd->select(str);
	}
}

/* connection hooks */

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	if (stacking) {
		object conn;
		object base;
		object user;
		string banner;
		mapping managers;
		int port;
		int timeout;

		conn = previous_object();

		stacking = 0;

		catch {
			banner = query_banner(conn);
		} : {
			banner = "Connection manager fault.\n";
			timeout = -1;
		}

		if (banner) {
			previous_object()->message(banner);
		}

		catch {
			if (!timeout) {
				timeout = query_timeout(conn);
			}
		} : {
			timeout = -1;
		}

		if (timeout < 0) {
			return MODE_DISCONNECT;
		}

		call_out("timeout", timeout, conn);

		return MODE_NOCHANGE;
	} else {
		previous_object()->message("Internal error: connection manager fault\n");
		return MODE_DISCONNECT;
	}
}

int receive_message(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	connection(previous_object());
	redirect(this_object()->select(str), str);

	return MODE_NOCHANGE;
}
