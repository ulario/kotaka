/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013, 2015  Raymond Jennings
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
#include <kernel/kernel.h>
#include <kernel/user.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;
inherit conn LIB_CONN;

object manager;

static void create(int clone)
{
	if (clone) {
		conn::create("telnet");
	}
}

void connect(string addr, int port)
{
	ACCESS_CHECK(SYSTEM());

	::connect(addr, port);
}

void set_manager(object new_manager)
{
	ACCESS_CHECK(SYSTEM());

	manager = new_manager;
}

object query_manager()
{
	ACCESS_CHECK(SYSTEM());

	return manager;
}

static void connect_failed(int refused)
{
	if (manager) {
		catch {
			manager->connect_failed(refused);
		}
	}

	destruct_object(this_object());
}

/*
 * NAME:	open()
 * DESCRIPTION:	open the connection
 */
static int open()
{
	::open(allocate(DRIVER->query_tls_size()));

	return FALSE;
}

/*
 * NAME:	close()
 * DESCRIPTION:	close the connection
 */
static void close(int dest)
{
	::close(allocate(DRIVER->query_tls_size()), dest);
}

/*
 * NAME:	timeout()
 * DESCRIPTION:	connection timed out
 */
static void timeout()
{
	::timeout(allocate(DRIVER->query_tls_size()));
}

/*
 * NAME:	receive_message()
 * DESCRIPTION:	forward a message to listeners
 */
static void receive_message(string str)
{
	::receive_message(allocate(DRIVER->query_tls_size()), str);
}

void set_mode(int mode)
{
	if (KERNEL() || SYSTEM()) {
		::set_mode(mode);
	}
}

/*
 * NAME:	message_done()
 * DESCRIPTION:	called when output is completed
 */
static void message_done()
{
	::message_done(allocate(DRIVER->query_tls_size()));
}

void send_message(string str)
{
	::send_message(str);
}
