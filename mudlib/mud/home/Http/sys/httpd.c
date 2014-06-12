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
		object header;

		TLSD->set_tls_value("Http", "connection-abort", 1);

		header = new_object("~/lwo/http_response");
		header->set_status(403, "Banned");

		return header->generate_header()
			+ read_file("~/data/error/403-banned.html");
	}

	return "";
}

string query_blocked_banner(object connection)
{
	object header;

	header = new_object("~/lwo/http_response");
	header->set_status(503, "Server suspended");

	return header->generate_header()
		+ read_file("~/data/error/503-blocked.html");
}

string query_overload_banner(object connection)
{
	object header;

	header = new_object("~/lwo/http_response");
	header->set_status(503, "Server busy");

	return header->generate_header()
		+ read_file("~/data/error/503-overload.html");
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
