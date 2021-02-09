/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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
#include <kotaka/paths/account.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit SECOND_AUTO;

static void unbind_telnet_ports()
{
	int sz;
	object this;

	this = this_object();

	for (sz = status(ST_ARRAYSIZE); --sz >= 0; ) {
		object manager;

		manager = SYSTEM_USERD->query_telnet_manager(sz);

		if (manager == this) {
			SYSTEM_USERD->set_telnet_manager(sz, nil);
		}
	}
}

static void unbind_binary_ports()
{
	int sz;
	object this;

	this = this_object();

	for (sz = status(ST_ARRAYSIZE); --sz >= 0; ) {
		object manager;

		manager = SYSTEM_USERD->query_binary_manager(sz);

		if (manager == this) {
			SYSTEM_USERD->set_binary_manager(sz, nil);
		}
	}
}

static void bind_telnet_port(int port)
{
	int sz;
	int *ports;

	ports = status(ST_TELNETPORTS);

	for (sz = sizeof(ports); --sz >= 0; ) {
		if (ports[sz] == port) {
			SYSTEM_USERD->set_telnet_manager(sz, this_object());
			return;
		}
	}

	error("No such telnet port " + port);
}

static void bind_binary_port(int port)
{
	int sz;
	int *ports;

	ports = status(ST_BINARYPORTS);

	for (sz = sizeof(ports); --sz >= 1; ) {
		if (ports[sz] == port) {
			SYSTEM_USERD->set_binary_manager(sz, this_object());
			return;
		}
	}

	error("No such binary port " + port);
}

string query_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return "Welcome to Kotaka\n";
}

string query_blocked_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return "The mud is undergoing maintenance.\n";
}

string query_overload_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return "The mud is too full to accept any more connections.\n";
}

string query_sitebanned_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return "Banned.\n";
}

void siteban_notify(object LIB_CONN connection)
{
	/* the connection is from a source that just got sitebanned and is about to be closed */
}

int query_timeout(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return 60;
}

object select(string str)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return find_object(SYSTEM_USERD);
}

static void connect(string host, int port)
{
	object conn;

	conn = clone_object(SYSTEM_CONN);
	conn->set_manager(this_object());
	conn->connect(host, port);
}
