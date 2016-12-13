/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>

inherit wiz "~/closed/lib/wiztool_gate";
inherit userd LIB_USERD;
inherit user LIB_USER;
inherit SECOND_AUTO;

string message;
mapping connections;
/* ([ connobj : ({ delay, trust, callout }) ]) */

static void create()
{
	wiz::create(0);
	userd::create();
	user::create();

	connections = ([ ]);

	call_out("configure", 0);
}

static void configure()
{
	SYSTEM_USERD->set_telnet_manager(1, this_object());
}

private float swap_used_ratio()
{
	return (float)status(ST_SWAPUSED) / (float)status(ST_SWAPSIZE);
}

static mixed message(string msg)
{
	message = msg;

	return nil;
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

string query_sitebanned_banner(object conn)
{
	return "Access denied.\n";
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
	conn->message("\033[0m\033[15;20r\033[15;1H");
}

private void prompt(object conn)
{
	conn->message("[\033[1;34mstatus\033[0m] ");
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
		SUSPENDD->queue_delayed_work("report", 0, conn)
	});

	redraw(conn);
	prompt(conn);

	return MODE_NOCHANGE;
}

private int printstatus(object conn)
{
	if (conn) {
		return conn->message("\0337\033[1;1H" + status_message() + "\n\0338");
	}
}

int receive_message(string str)
{
	object conn;
	string *params;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	if (!connections[conn]) {
		return MODE_DISCONNECT;
	}

	ASSERT(str);

	conn = previous_object();

	params = explode(str, " ") - ({ "" });

	if (sizeof(params)) {
		switch(params[0]) {
		case "redraw":
			redraw(conn);
			break;

		case "quit":
			SUSPENDD->dequeue_delayed_work(connections[conn][2]);
			conn->message("\033c");
			return MODE_DISCONNECT;

		case "interval":
			if (sizeof(params) < 2) {
				conn->message("Usage: interval <interval>\n");
				break;
			} else {
				float interval;

				sscanf(params[1], "%f", interval);

				if (interval < 15.0 && !connections[conn][1]) {
					conn->message("Intervals less than 15 seconds\nare only allowed for local connections.\n");
					break;
				}

				connections[conn][0] = interval;

				if (connections[conn][2]) {
					SUSPENDD->dequeue_delayed_work(connections[conn][2]);
					connections[conn][2] = SUSPENDD->queue_delayed_work("report", interval, conn);
				}
			}

		case "":
			break;

		default:
			conn->message("Commands: interval, quit, redraw\n");
			break;
		}

	}

	prompt(conn);

	return MODE_NOCHANGE;
}

int message_done()
{
	object conn;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	conn = previous_object();

	if (!connections[conn]) {
		return MODE_DISCONNECT;
	}

	if (!connections[conn][2]) {
		connections[conn][2] = SUSPENDD->queue_delayed_work("report",
			connections[conn][0], conn
		);
	}

	return MODE_NOCHANGE;
}

void report(object conn)
{
	int status;

	ACCESS_CHECK(previous_program() == SUSPENDD);

	if (!conn) {
		return;
	}

	if (!connections[conn]) {
		return;
	}

	connections[conn][2] = 0;

	printstatus(conn);
}
