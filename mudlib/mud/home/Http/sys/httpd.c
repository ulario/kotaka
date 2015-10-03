/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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

string generate_error_page(int status_code, string status_message, string lines...)
{
	string response;
	object header;
	int i, sz;

	header = new_object("~/lwo/http_response");
	header->set_status(status_code, status_message);

	response = header->generate_header();

	response += "<html>\n";
	response += "<head>\n";
	response += "<title>" + status_message + "</title>\n";
	response += "</head>\n";

	response += "<body>\n";
	response += "<h1 style=\"color: red\">" + status_message + "</h1>\n";

	for (sz = sizeof(lines); i < sz; i++) {
		response += "<p>" + lines[i] + "</p>\n";
	}

	response += "</body>\n";
	response += "</html>\n";

	return response;
}

/* hooks */

string query_banner(object connection)
{
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

string query_sitebanned_banner(object connection)
{
	object header;

	header = new_object("~/lwo/http_response");
	header->set_status(403, "Banned");

	return header->generate_header()
		+ read_file("~/data/error/403-banned.html");
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
