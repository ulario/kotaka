/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020  Raymond Jennings
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
#include <kotaka/paths/account.h>
#include <kotaka/paths/thing.h>
#include <kotaka/paths/http.h>
#include <kotaka/paths/system.h>
#include <kernel/user.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>
#include <type.h>

#define STATE_HEADERS    1
#define STATE_ENTITY     2
#define STATE_RESPONDING 3

inherit LIB_SYSTEM_USER;
inherit "/lib/sort";
inherit "~/lib/object";
inherit "~/lib/siteban";
inherit "~/lib/style";

string ip;
string request;
int state;
mapping headers;

string method;
string path;
string version;

string entity;
int explen;
int receiving_entity;

private void handle_get_object(string objectname)
{
	mixed obj;

	obj = string2object(objectname);

	if (typeof(obj) == T_OBJECT) {
		object header;

		header = new_object("~/lwo/http_response");
		header->set_status(200, "Object report");

		message(header->generate_header());
		message("<html>\n");
		message("<head>\n");
		message(style());
		message("<title>Object report</title>\n");
		message("</head>\n");
		message("<body>\n");
		message("<div class=\"column\">\n");
		message(object_text(obj));
		message("</div>\n");
		message("</body>\n");
		message("</html>\n");
	} else {
		message(HTTPD->generate_error_page(404, "No such object", "That object does not appear to exist"));
	}
}

private void handle_get_destruct(string objectname)
{
	mixed obj;

	if (ip != "127.0.0.1") {
		message(HTTPD->generate_error_page(403, "Forbidden", "Access denied"));
		return;
	}

	obj = string2object(objectname);

	if (typeof(obj) == T_OBJECT) {
		object env;
		object header;

		env = obj->query_environment();
		obj->self_destruct();

		header = new_object("~/lwo/http_response");
		header->set_status(200, "Object destroyed");

		message(header->generate_header());
		message("<html>\n");
		message("<head>\n");
		message(style());
		message("<title>Object destroyed</title>\n");
		message("</head>\n");
		message("<body>\n");
		message("<p>Object destroyed</p>\n");
		message("</body>\n");
		message("</html>\n");
	} else {
		message(HTTPD->generate_error_page(404, "No such object", "That object does not appear to exist"));
	}
}

private int handle_get_pattern(string path)
{
	string handler;
	string args;

	if (sscanf(path, "/%s.lpc", handler)) {
		switch(handler) {
		case "object":
			if (sscanf(path, "/object.lpc?obj=%s", args)) {
				handle_get_object(urldecode(args));
				return 1;
			} else {
				message(HTTPD->generate_error_page(400, "Bad Request", "<p>Malformed arguments</p>"));
				return 1;
			}
			break;

		case "destruct":
			if (sscanf(path, "/destruct.lpc?obj=%s", args)) {
				handle_get_destruct(urldecode(args));
				return 1;
			} else {
				message(HTTPD->generate_error_page(400, "Bad Request", "<p>Malformed arguments</p>"));
				return 1;
			}

		case "siteban":
			if (path == "/siteban.lpc") {
				handle_get_siteban();
				return 1;
			} else {
				message(HTTPD->generate_error_page(400, "Bad Request", "<p>Malformed arguments</p>"));
				return 1;
			}
			break;
		}
	}
}

private void handle_post_object(string objectname)
{
	mixed obj;

	obj = string2object(objectname);

	if (typeof(obj) == T_OBJECT) {
		object header;
		float new_mass;

		header = new_object("~/lwo/http_response");
		header->set_status(200, "Object report");

		message(header->generate_header());
		message("<html>\n");
		message("<head>\n");
		message(style());
		message("<title>Object report</title>\n");
		message("</head>\n");
		message("<body>\n");
		if (sscanf(entity, "localmass=%f", new_mass)) {
			obj->set_local_mass(new_mass);
			message("<h1>Changes</h1>\n");
			message("<p>Local mass changed to " + new_mass + "</p>\n");
			message("<p>New object mass: " + obj->query_mass() + " kg</p>\n");
		}
		message(object_text(obj));
		message("<h1>Returned entity</h1>\n");
		message("<pre>\n");
		message(entity);
		message("</pre>\n");
		message("</body>\n");
		message("</html>\n");
	} else {
		object header;

		header = new_object("~/lwo/http_response");
		header->set_status(404, "No such object");

		message(header->generate_header());
		message("<html>\n");
		message("<head>\n");
		message("<title>No such object</title>\n");
		message("</head>\n");
		message("<body>\n");
		message("<h1 style=\"color: red\">No such object</h1>\n");
		message("<p>" + obj + "</p>");
		message("</body>\n");
		message("</html>\n");
	}
}

private int handle_post_pattern(string path)
{
	string handler;
	string args;

	if (sscanf(path, "/%s.lpc", handler)) {
		switch(handler) {
		case "object":
			if (sscanf(path, "/object.lpc?obj=%s", args)) {
				handle_post_object(args);
				return 1;
			} else {
				object header;

				header = new_object("~/lwo/http_response");
				header->set_status(400, "Invalid format");

				message(header->generate_header());
				message("Malformed arguments");
				return 1;
			}
		}
	}
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

private string nohandler_text()
{
	object header;
	string buffer;
	object conn;

	header = new_object("~/lwo/http_response");
	header->set_status(503, "No handler");

	buffer = header->generate_header();

	buffer += "<html>\n";
	buffer += "<head>\n";
	buffer += "<title>No handler</title>\n";
	buffer += "</head>\n";
	buffer += "<body>\n";
	buffer += "<h1 style=\"color: red\">No handler</h1>\n";
	buffer += "<table>\n";
	buffer += "<tr><td>Method</td><td>" + method + "</td></tr>\n";
	buffer += "<tr><td>Path</td><td>" + path + "</td></tr>\n";
	buffer += "<tr><td>Version</td><td>" + version + "</td></tr>\n";
	buffer += "</table>\n";
	buffer += "<p>There is no handler for that path.</p>\n";
	buffer += "<p>For the curious, here's a list of all objects involved in this connection:</p>\n";

	conn = this_object();

	buffer += "<p style=\"color: darkgreen\">\n";

	while(conn) {
		buffer += object_name(conn) + "<br />\n";

		if (conn <- LIB_USER) {
			conn = conn->query_conn();
		} else {
			break;
		}
	}

	buffer += "</p>\n";
	buffer += "<p>And here is the request your browser sent:</p>";
	buffer += "<pre>\n";
	buffer += request;
	buffer += "</pre>\n";
	buffer += "</body>\n";
	buffer += "</html>\n";

	return buffer;
}

private void handle_get()
{
	catch {
		if (handle_get_pattern(path)) {
			return;
		}

		message(nohandler_text());
	} : {
		message(HTTPD->generate_error_page(503, "Internal server error", "Unspecified server error."));
	}
}

private void handle_post()
{
	if (ip != "127.0.0.1") {
		message(HTTPD->generate_error_page(403, "Access denied", "Privileged IP required"));
		return;
	}

	catch {
		if (handle_post_pattern(path)) {
			return;
		}

		message(nohandler_text());
	} : {
		message(HTTPD->generate_error_page(503, "Internal server error", "Unspecified server error."));
	}
}

/**********/
/* static */
/**********/

static void create(int clone)
{
	explen = -1;
	state = -1;

	if (clone) {
		call_out("self_destruct", 5);
	}
}

static string query_ip()
{
	return ip;
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

/**********/
/* public */
/**********/

/* user hooks */

int login(string str)
{
	object conn;

	conn = previous_object();

	connection(conn);

	while (conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	ip = query_ip_number(conn);

	write_file("http.log", ip + ": " + str + "\n");

	request = str + "\n";

	if (sscanf(str, "%s %s %s", method, path, version) != 3) {
		message(HTTPD->generate_error_page(400, "Bad request"
			, "The request does not appear to be a valid HTTP/1.1 request.")
		);

		return MODE_DISCONNECT;
	}

	headers = ([ ]);

	state = STATE_HEADERS;

	return MODE_NOCHANGE;
}

void logout(int quit)
{
	destruct_object(this_object());
}

int receive_message(string message)
{
	switch(state) {
	case STATE_HEADERS:
		{
			string name, value;

			if (message == "") {
				switch(method) {
				case "GET":
					state = STATE_RESPONDING;
					handle_get();
					call_out("self_destruct", 0.05);
					return MODE_BLOCK;

				case "POST":
					state = STATE_ENTITY;
					entity = "";
					set_mode(MODE_RAW);

					{
						string cl;

						cl = headers["Content-Length"];

						if (!cl) {
							message(HTTPD->generate_error_page(400, "Bad request"
								, "POST sent without a Content-Length header.")
							);
							return MODE_DISCONNECT;
						}

						if (!sscanf(cl, "%d", explen)) {
							message(HTTPD->generate_error_page(400, "Bad request"
								, "POST sent with malformed Content-Length header.")
							);
							return MODE_DISCONNECT;
						}
					}

					if (explen == 0) {
						/* empty entity, process immediately */
						handle_post();
						call_out("self_destruct", 0.05);
						return MODE_BLOCK;
					}

					/* wait for entity */
					return MODE_NOCHANGE;
				}
			}

			if (sscanf(message, "%s: %s", name, value) != 2) {
				message(HTTPD->generate_error_page(400, "Bad request"
					, "Your browser sent malformed headers.")
				);
				call_out("self_destruct", 0.05);
				return MODE_BLOCK;
			}

			headers[name] = value;
		}
		return MODE_NOCHANGE;

	case STATE_ENTITY:
		ASSERT(explen > 0);

		entity += message;

		if (strlen(entity) > explen) {
			message(HTTPD->generate_error_page("400", "Overflow", "Sent entity size exceeds Content-Length header"));
			return MODE_DISCONNECT;
		}

		if (strlen(entity) == explen) {
			handle_post();
			return MODE_DISCONNECT;
		}

		return MODE_NOCHANGE;
	}
}

int message_done()
{
	return MODE_NOCHANGE;
}
