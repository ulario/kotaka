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
#include <kotaka/paths/thing.h>
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

private void do_status()
{
}

private string simplename(object obj)
{
	string name;

	name = obj->query_object_name();

	if (name) {
		return name;
	} else {
		return object_name(obj);
	}
}

private string object2string(object obj)
{
	string name;

	name = "";

	if (obj <- LIB_THING) {
		object env;
		string id;

		name = obj->query_object_name();

		if (name) {
			return name;
		}

		env = obj->query_environment();

		if (env) {
			id = obj->query_id();

			if (id) {
				return object2string(env) + ";" + id;
			} else {
				return simplename(obj);
			}
		} else {
			return simplename(obj);
		}
	} else {
		return simplename(obj);
	}
}

private void do_thing(object obj)
{
	mapping lprops;
	object *archs;
	object env;
	object *inv;
	int sz;

	archs = obj->query_archetypes();
	sz = sizeof(archs);

	if (sz) {
		int i;

		message("<p>Archetypes:</p>\n");
		message("<ul>\n");

		for (i = 0; i < sz; i++) {
			message("<li>" + object2string(archs[i]) + "</li>\n");
		}

		message("</ul>\n");
	}

	env = obj->query_environment();

	if (env) {
		message("<p>Environment: " + object2string(env) + "</p>\n");
	}

	inv = obj->query_inventory();
	sz = sizeof(inv);

	if (sz) {
		int i;

		message("<p>Inventory:</p>\n");
		message("<ul>\n");

		for (i = 0; i < sz; i++) {
			message("<li>" + object2string(inv[i]) + "</li>\n");
		}

		message("</ul>\n");
	}

	
}

private void handle_get_object(string objectname)
{
	object header;
	object obj;

	obj = find_object(objectname);

	if (!obj) {
		obj = CATALOGD->lookup_object(objectname);
	}

	if (obj) {
		header = new_object("~/lwo/http_response");

		header->set_status(200, "Object report");

		message(header->generate_header());
		message("<html>\n");
		message("<head>\n");
		message("<title>Object report</title>\n");
		message("</head>\n");
		message("<body>\n");
		message("<h1 style=\"color: green\">Object report</h1>\n");
		message("<p>Object name: " + objectname);
		message("<p>Object owner: " + obj->query_owner());

		if (obj <- LIB_THING) {
			do_thing(obj);
		}

		message("</body>\n");
		message("</html>\n");
	} else {
		header = new_object("~/lwo/http_response");

		header->set_status(404, "No such object");

		message(header->generate_header());
		message("<html>\n");
		message("<head>\n");
		message("<title>No such object</title>\n");
		message("</head>\n");
		message("<body>\n");
		message("<h1 style=\"color: red\">No handler</h1>\n");
		message("<p>" + objectname + " does not exist.");
		message("</body>\n");
		message("</html>\n");
	}
}

private int handle_get_pattern(string path)
{
	string a, b;

	if (sscanf(path, "/%s-%s", a, b) && a == "object") {
		handle_get_object(b);
		return 1;
	}

	return 0;
}

private void do_formtest()
{
	object header;

	header = new_object("~/lwo/http_response");

	header->set_status(200, "Ok");

	message(header->generate_header());

	message("<html>\n");
	message("<head>\n");
	message("<title>A test form.</title>\n");
	message("</head>\n");
	message("<body>\n");
	message("<h1>Form test</h1>\n");
	message("<form action=\"/formtest\" method=\"post\">\n");
	message("<p>Test box: <input type=\"text\" name=\"testbox\"></p>");
	message("<input type=\"submit\" name=\"Send\">");
	message("</form>\n");
	message("</body>\n");
	message("</html>\n");
}

private void do_formtest_post()
{
}

private void handle_get()
{
	catch {
		object conn;
		object header;

		switch(path) {
		case "/status":
			do_status();
			return;

		case "/formtest":
			do_formtest();
			return;

		}

		if (handle_get_pattern(path)) {
			return;
		}

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

		default:
			message(
				HTTPD->generate_error_page(
					500, "Bad request",
					"This server can't handle the " + method + " method yet.\n"
				)
			);
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
