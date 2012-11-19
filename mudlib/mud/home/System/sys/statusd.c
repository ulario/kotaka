/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <status.h>

inherit wiz "~/closed/lib/wiztool_gate";
inherit userd LIB_USERD;
inherit user LIB_USER;
inherit SECOND_AUTO;

string message;
mapping connections;
float interval;
/* ([ connobj : ({ delay, trust, callout }) ]) */

static void create()
{
	wiz::create(0);
	userd::create();
	user::create();

	connections = ([ ]);
	interval = 0.0;
}

void disable()
{
	ACCESS_CHECK(KADMIN() || SYSTEM());

	SYSTEM_USERD->set_telnet_manager(1, nil);
}

void enable()
{
	ACCESS_CHECK(KADMIN() || SYSTEM());

	SYSTEM_USERD->set_telnet_manager(1, this_object());
}

private float swap_used_ratio()
{
	return (float)status(ST_SWAPUSED) / (float)status(ST_SWAPSIZE);
}

mixed message(string msg)
{
	switch (previous_program()) {
	case LIB_WIZTOOL:
		message = msg;
		return nil;
	default:
		error(previous_program() + " is harassing me!");
	}
}

string status_message()
{
	cmd_status(nil, nil, nil);
	return message;
}

/* I/O stuff */

string query_banner(object conn)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return nil;
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

private void redraw(object conn)
{
	conn->message("\033c\033[2J");
	conn->message("\033[14;1H\033[1m-----------------------------------------");
	conn->message("\033[21;1H-----------------------------------------");
	conn->message("\033[21m\033[15;20r\033[15;1H");
}

private void prompt(object conn)
{
	conn->message("[\033[1;34mstatus\033[21;37m] ");
}

int login(string str)
{
	object base_conn, conn;

	int trusted;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	ASSERT(str == nil);

	conn = previous_object();
	base_conn = conn;

	while (base_conn && base_conn <- LIB_USER) {
		base_conn = base_conn->query_conn();
	}

	switch(query_ip_number(base_conn)) {
	case "::1":
	case "127.0.0.1":
		trusted = 1;
		break;
	}

	connections[conn] = ({
		trusted ? 0.05 : 15.0,
		trusted,
		call_out("report", 0, conn)
	});

	redraw(conn);
	prompt(conn);

	return MODE_NOCHANGE;
}

private int printstatus(object conn)
{
	if (conn) {
		return conn->message("\0337\033[1;1H" + status_message() + "\n\0338");
	} else {
		LOGD->post_message("status", LOG_INFO, status_message());
		return 0;
	}
}

int receive_message(string str)
{
	object conn;
	string *params;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	if (!str) {
		catch(error("suspicious"));
		return MODE_NOCHANGE;
	}

	conn = previous_object();

	params = explode(str, " ") - ({ "" });

	if (sizeof(params)) switch(params[0]) {
	case "redraw":
		redraw(conn);
		break;
	case "quit":
		remove_call_out(connections[conn][2]);
		conn->message("\033c");
		return MODE_DISCONNECT;
	case "interval":
		if (sizeof(params) < 1) {
			conn->message("Usage: interval <interval>\n");
			break;
		} else {
			float interval;

			sscanf(params[1], "%f", interval);

			if (interval < 15.0 && !connections[conn][1]) {
				conn->message("Intervals less than 15 seconds\nare only allowed for local connections.\n");
				break;
			}

			remove_call_out(connections[conn][2]);

			connections[conn][0] = interval;
			connections[conn][2] = call_out("report", interval, conn);
		}
	case "":
		break;
	default:
		conn->message("Commands: interval, quit, redraw\n");
		break;
	} else {
		printstatus(conn);
	}

	prompt(conn);

	return MODE_NOCHANGE;
}

int message_done()
{
	object conn;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	conn = previous_object();

	if (!connections[conn][2]) {
		connections[conn][2] = call_out("report",
			connections[conn][0], conn
		);
	}

	return MODE_NOCHANGE;
}

static void report(object conn)
{
	int status;

	if (!conn) {
		return;
	}

	connections[conn][2] = 0;

	status = printstatus(conn);

	if (status) {
		connections[conn][2] = call_out("report",
			connections[conn][0], conn
		);
	}
}
