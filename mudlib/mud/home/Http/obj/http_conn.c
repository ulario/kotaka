/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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

inherit LIB_SYSTEM_USER;

static void create(int clone)
{
}

private int input(string message);

int login(string str)
{
	connection(previous_object());

	return input(str);
}

int receive_message(string message)
{
	return input(message);
}

void logout(int quit)
{
	destruct_object(this_object());
}

int message_done()
{
	return MODE_DISCONNECT;
}

private int input(string message)
{
	if (message == "") {
		object conn;
		object header;

		header = new_object("~/lwo/http_response");

		header->set_status(503, "No handler");

		message(header->generate_header());
		message("<html>\n");
		message("<head>\n");
		message("<title>No handler</title>\n");
		message("</head>\n");
		message("<body>\n");
		message("<h1 style=\"color: red\">No handler</h1>\n");
		message("<p>There is no handler for that path.</p>\n");
		message("<p>For the curious, here's a list of all objects involved in this connection:</p>\n");

		conn = this_object();

		message("<p style=\"color: darkgreen\">\n");

		while(conn) {
			message(object_name(conn) + "<br />\n");

			if (conn <- LIB_USER) {
				conn = conn->query_conn();
			} else {
				break;
			}
		}

		message("</p>\n");
		message("</body>\n");
		message("</html>\n");
		return MODE_DISCONNECT;
	}

	return MODE_NOCHANGE;
}
