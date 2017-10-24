/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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

mapping delay; /* ([ obj: delay ]) */
mapping handle; /* ([ obj: handle ]) */
mapping ready; /* ([ obj: is_ready ]) */
mapping pending; /* ([ obj: is_pending ]) */
mapping last; /* ([ obj: last millitime ]) */

static void create()
{
	message = "";

	delay = ([ ]);
	handle = ([ ]);
	ready = ([ ]);
	pending = ([ ]);
	last = ([ ]);

	wiz::create(0);
	userd::create();
	user::create();

	call_out("configure", 0);
}

void upgrade()
{
	ACCESS_CHECK(SYSTEM());

	delay = ([ ]);
	handle = ([ ]);
	ready = ([ ]);
	pending = ([ ]);
}

static void configure()
{
	SYSTEM_USERD->set_telnet_manager(0, this_object());
}

static mixed message(string msg)
{
	message += msg;

	return nil;
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

private void print_report(object conn)
{
	int status;
	string *lines;
	int i, sz;

	message = "";

	message += "Status:\n";
	cmd_status(nil, nil, nil);

	message += "\nResources:\n";
	cmd_rsrc(nil, nil, nil);

	message += "\nTick usage:\n";
	cmd_rsrc(nil, nil, "tick usage");

	message += "\nCallout usage:\n";
	cmd_rsrc(nil, nil, "callout usage");

	if (last[conn]) {
		mixed *now;
		float time;

		now = millitime();

		time = (float)now[0] + now[1];
		time -= (float)last[conn][0] + last[conn][1];

		message += "Delay since last report: " + time + "\n";
	}

	last[conn] = millitime();

	lines = explode(message, "\n");
	sz = sizeof(lines);

	conn->message("\033[1;1H");

	for (sz = sizeof(lines), i = 0; i < sz; i++) {
		conn->message(lines[i] + "\033[K\n");
	}

	conn->message("\033[J");
}

private void schedule(object conn)
{
	mixed **callouts;
	int sz;

	callouts = SUSPENDD->query_callouts();

	for(sz = sizeof(callouts); --sz >= 0; ) {
		mixed *callout;

		callout = callouts[sz];

		if (callout[CO_FUNCTION] != "report") {
			continue;
		}

		if (callout[CO_FIRSTXARG] == conn) {
			SUSPENDD->dequeue_delayed_work(callout[CO_HANDLE]);
		}
	}

	if (delay[conn] == nil) {
		delay[conn] = is_trusted(conn) ? 0.05 : 1.0;
	}

	handle[conn] = SUSPENDD->queue_delayed_work("report", delay[conn], conn);
}

void report(object conn)
{
	ACCESS_CHECK(SYSTEM());

	if (!conn) {
		return;
	}

	handle[conn] = nil;

	if (ready && ready[conn]) {
		ready[conn] = nil;

		print_report(conn);

		schedule(conn);
	} else {
		if (!pending) {
			pending = ([ ]);
		}

		pending[conn] = 1;

		return;
	}
}

static void clear(object conn)
{
	if (!conn) {
		return;
	}

	conn->message("\033[1;1H\033[2J");
}

/* userd hooks */

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

/* user hooks */

int login(string str)
{
	float delay;
	object conn;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	if (str) {
		return MODE_DISCONNECT;
	}

	conn = previous_object();

	if (is_trusted(conn)) {
		delay = 0.05;
	} else {
		delay = 1.0;
	}

	handle[conn] = SUSPENDD->queue_delayed_work("report", delay / 2.0, conn);

	conn->message("\033[1;1H\033[2J");

	return MODE_NOECHO;
}

int receive_message(string str)
{
	object conn;
	string *params;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	if (!str) {
		return MODE_DISCONNECT;
	}

	conn = previous_object();
	ASSERT(conn <- LIB_CONN);

	params = explode(str, " ") - ({ "" });

	if (sizeof(params)) {
		switch(params[0]) {
		case "clear":
			call_out("clear", 0, conn);
			break;

		case "quit":
			if (handle[conn]) {
				SUSPENDD->dequeue_delayed_work(handle[conn]);
				handle[conn] = nil;
			}
			conn->message("\033c");
			return MODE_DISCONNECT;

		case "interval":
			if (sizeof(params) < 2) {
				conn->message("Usage: interval <interval>\n");
				call_out("clear", 5.0, conn);
				break;
			} else {
				float interval;

				sscanf(params[1], "%f", interval);

				if (interval < 1.0 && !is_trusted(conn)) {
					conn->message("Intervals less than 1 second\nare only allowed for local connections.\n");
					call_out("clear", 5.0, conn);
					break;
				}

				delay[conn] = interval;

				if (handle[conn]) {
					SUSPENDD->dequeue_delayed_work(handle[conn]);
					schedule(conn);
				}
			}
			break;

		default:
			conn->message("Commands: clear, interval, quit\n");
			call_out("clear", 5.0, conn);
			break;
		}
	} else {
		call_out("clear", 0, conn);
	}

	return MODE_NOCHANGE;
}

int message_done()
{
	object conn;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	conn = previous_object();

	if (pending && pending[conn]) {
		pending[conn] = nil;
		print_report(conn);

		schedule(conn);
	} else {
		ready[conn] = 1;
	}

	return MODE_NOCHANGE;
}
