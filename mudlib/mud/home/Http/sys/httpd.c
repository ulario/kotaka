/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kernel/user.h>

inherit LIB_USERD;

static void create()
{
	SYSTEM_USERD->set_binary_manager(2, this_object());
}

string query_banner(object connection)
{
	object conn;

	conn = connection;

	while (conn && conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	if (is_sitebanned(query_ip_number(conn))) {
		TLSD->set_tls_value("Http", "connection-abort", 1);

		return "HTTP/1.1 403 Banned\n"
		+ "Connection: close\n\n"
		+ read_file("~/data/error/403-banned");
	}

	return "";
}

string query_blocked_banner(object connection)
{
	return "HTTP/1.1 500 Server suspended\n"
	+ "Connection: close\n\n"
	+ read_file("~/data/error/500-blocked");
}

string query_overload_banner(object connection)
{
	return "HTTP/1.1 500 Server busy\n"
	+ "Connection: close\n\n"
	+ read_file("~/data/error/500-overload");
}

int query_timeout(object connection)
{
	if (TLSD->query_tls_value("Http", "connection-abort")) {
		return -1;
	}

	return 5;
}

object select(string str)
{
	return clone_object("../obj/http_conn");
}
