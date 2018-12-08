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
#include <kernel/user.h>
#include <kotaka/assert.h>
#include <kotaka/paths/account.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>

inherit wiz "~/closed/lib/wiztool_gate";
inherit userd LIB_USERD;
inherit user LIB_USER;
inherit SECOND_AUTO;

string message;
mapping connections; /* ([ connection : ({ interval, handle }) ]) */

private void schedule(object conn)
{
	int handle;
	float interval;

	({ interval, handle }) = connections[conn];

	if (handle) {
		remove_call_out(handle);
	}

	connections[conn] = ({ interval, call_out("report", interval, conn) });
}

private void wipe_callouts()
{
	mixed **callouts;
	int sz;

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		remove_call_out(callouts[sz][CO_HANDLE]);
	}
}

private int is_trusted(object conn)
{
	while (conn && conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	switch(query_ip_number(conn)) {
	case "::1":
	case "127.0.0.1":
		return 1;
	default:
		return 0;
	}
}

private string print_report()
{
	int status;
	string *lines;
	int i, sz;
	object conn;

	conn = query_conn();

	message = "";

	message += "Status:\n";
	cmd_status(nil, nil, nil);

	message += "\nResources:\n";
	cmd_rsrc(nil, nil, nil);

	return message;
}

static void create()
{
	message = "";
	connections = ([ ]);

	wiz::create(0);
	userd::create();
	user::create();

	load_object(SYSTEM_USERD);

	SYSTEM_USERD->set_telnet_manager(0, this_object());
}

static mixed message(string msg)
{
	message += msg;

	return nil;
}

static void report(object conn)
{
	string message;
	string *lines;
	int sz, i;

	if (!conn) {
		return;
	}

	if (!connections[conn]) {
		return;
	}

	connections[conn][1] = 0;

	message = print_report();

	lines = explode(message, "\n");

	conn->message("\033[1;1H");

	for (sz = sizeof(lines), i = 0; i < sz; i++) {
		conn->message(lines[i] + "\033[K\n");
	}

	conn->message("\033[J");

	schedule(conn);
}

static void clear(object conn)
{
	if (!conn) {
		return;
	}

	conn->message("\033[1;1H\033[2J");
}

void upgrade()
{
	object *conns;
	int handle;
	float interval;
	int sz;

	ACCESS_CHECK(previous_program() == OBJECTD);

	wipe_callouts();

	if (!connections) {
		connections = ([ ]);
	}

	conns = map_indices(connections);

	for (sz = sizeof(conns); --sz >= 0; ) {
		object conn;

		conn = conns[sz];

		({ interval, handle }) = connections[conn];

		handle = call_out("report", interval, conn);

		connections[conn] = ({ interval, handle });
	}
}

string query_banner(object conn)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return nil;
}

string query_sitebanned_banner(object connection)
{
	string ip;
	mapping ban;
	string output;
	string message;
	mixed expire;

	while (connection && connection <- LIB_USER) {
		connection = connection->query_conn();
	}

	ip = query_ip_number(connection);

	ban = BAND->check_siteban(ip);

	if (!ban) {
		return "Access denied";
	}

	output = "Access denied";

	if (message = ban["message"]) {
		output += " (" + message + ")";
	}

	expire = ban["expire"];

	if (expire != nil && expire != -1) {
		int remaining;

		remaining = expire - time();

		if (remaining < 60) {
			output += " (expires in " + remaining + " seconds)";
		} else if (remaining < 3600) {
			output += " (expires in " + (remaining / 60 + 1) + " minutes)";
		} else if (remaining < 86400) {
			output += " (expires in " + (remaining / 3600 + 1) + " hours)";
		} else {
			output += " (expires in " + (remaining / 86400 + 1) + " days)";
		}
	}

	return output + "\n";
}

int query_timeout(object conn)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	connection(conn);
	redirect(this_object(), nil);

	return 0;
}

object select(string input)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return this_object();
}

int login(string str)
{
	float interval;
	int handle;
	object conn;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	if (str) {
		LOGD->post_message("system", LOG_WARNING, "StatusD login line not nil");

		return MODE_DISCONNECT;
	}

	conn = previous_object();

	if (is_trusted(conn)) {
		interval = 0.05;
	} else {
		interval = 5.0;
	}

	connections[conn] = ({ interval, 0 });

	conn->message("\033[1;1H\033[2J");

	schedule(conn);

	return MODE_NOECHO;
}

void logout(int quit)
{
	object conn;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	conn = previous_object();

	if (connections[conn]) {
		float interval;
		int handle;

		({ interval, handle }) = connections[conn];

		remove_call_out(handle);

		connections[conn] = ({ -1, nil });
	}
}

int receive_message(string str)
{
	object conn;
	string *params;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	if (!str) {
		LOGD->post_message("system", LOG_WARNING, "Spurious receive_message, terminating connection");

		return MODE_DISCONNECT;
	}

	conn = previous_object();
	params = explode(str, " ") - ({ "" });

	if (sizeof(params)) {
		switch(params[0]) {
		case "clear":
			conn->message("\033c");
			break;

		case "quit":
			conn->message("\033c");
			return MODE_DISCONNECT;

		case "interval":
			if (sizeof(params) < 2) {
				conn->message("Usage: interval <interval>\n");
				break;
			} else {
				float interval;
				int handle;

				sscanf(params[1], "%f", interval);

				if (interval < 1.0 && !is_trusted(conn)) {
					conn->message("Intervals less than 1 second are only allowed for local connections.\n");
					break;
				}

				connections[conn][0] = interval;
				schedule(conn);
			}
			break;

		default:
			conn->message("Commands: clear, interval, quit\n");
			break;
		}
	} else {
		call_out("clear", 0, conn);
	}

	return MODE_NOCHANGE;
}

int message_done()
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	return MODE_NOCHANGE;
}
