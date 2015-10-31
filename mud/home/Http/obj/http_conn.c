/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2007, 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/http.h>
#include <kotaka/paths/system.h>
#include <kernel/user.h>

inherit LIB_SYSTEM_USER;

string request;

string method;
string path;
string version;

static void create(int clone)
{
	if (clone) {
		call_out("self_destruct", 5);
	}
}

private int input(string message);

int login(string str)
{
	connection(previous_object());

	request = str + "\n";

	if (sscanf(str, "%s %s %s", method, path, version) != 3) {
		message(HTTPD->generate_error_page(400, "Bad request"
			, "The request does not appear to be a valid HTTP/1.1 request.")
		);

		return MODE_DISCONNECT;
	}

	return input(str);
}

int receive_message(string message)
{
	request += message + "\n";

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

private void handle_get()
{
	catch {
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
		message("<table>\n");
		message("<tr><td>Method</td><td>" + method + "</td></tr>\n");
		message("<tr><td>Path</td><td>" + path + "</td></tr>\n");
		message("<tr><td>Version</td><td>" + version + "</td></tr>\n");
		message("</table>\n");
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
		message("<p>And here is the request your browser sent:</p>");
		message("<pre>\n");
		message(request);
		message("</pre>\n");
		message("</body>\n");
		message("</html>\n");
	} : {
		message(HTTPD->generate_error_page(503, "Internal server error", "Unspecified server error."));
	}
}

private int input(string message)
{
	if (message == "") {
		switch(method) {
		case "GET":
			handle_get();
			break;

		}

		return MODE_DISCONNECT;
	}

	return MODE_NOCHANGE;
}

static void self_destruct()
{
	if (query_conn()) {
		disconnect();
	}

	if (this_object()) {
		destruct_object(this_object());
	}
}
