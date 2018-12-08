/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/account.h>
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
	response += "<div style=\"text-align: center;\">\n";
	response += "<h1 style=\"color: red;\">" + status_message + "</h1>\n";

	for (sz = sizeof(lines); i < sz; i++) {
		response += "<p>" + lines[i] + "</p>\n";
	}

	response += "</div>\n";
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
	return generate_error_page(503
		, "Server suspended"
		, "The server is suspended at the moment due to internal maintenance."
		, "Please try again in 20 seconds."
	);
}

float query_blocked_timeout(object connection)
{
	return 0.1;
}

string query_overloaded_banner(object connection)
{
	return generate_error_page(503
		, "Server busy"
		, "The server has too many connections at the moment."
		, "Please try again in 60 seconds."
	);
}

float query_overloaded_timeout(object connection)
{
	return 0.1;
}

string query_sitebanned_banner(object connection)
{
	string ip;
	mapping ban;
	string *bits;

	while (connection && connection <- LIB_USER) {
		connection = connection->query_conn();
	}

	ip = query_ip_number(connection);

	ASSERT(ip);

	LOGD->post_message("system", LOG_NOTICE, "HTTP connection from banned ip " + ip);

	ban = BAND->check_siteban(ip);

	ASSERT(ban);

	bits = ({ "Banned", "You are banned from this server" });

	if (ban) {
		string message;
		mixed expire;

		message = ban["message"];

		if (ban["message"]) {
			bits += ({ message });
		}

		expire = ban["expire"];

		if (expire != nil) {
			int remaining;
			string emessage;

			remaining = expire - time();

			if (remaining < 60) {
				emessage = "Expires in " + remaining + " seconds";
			} else if (remaining < 3600) {
				emessage = "Expires in " + (remaining / 60 + 1) + " minutes";
			} else if (remaining < 86400) {
				emessage = "Expires in " + (remaining / 3600 + 1) + " hours";
			} else {
				emessage = "Expires in " + (remaining / 86400 + 1) + " days";
			}

			bits += ({ emessage });
		}
	}

	return generate_error_page(403, bits...);
}

float query_sitebanned_timeout(object connection)
{
	return 0.1;
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
