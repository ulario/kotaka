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
#include <kernel/kernel.h>
#include <kernel/user.h>
#include <kotaka/paths/utility.h>

static string whoami()
{
	object user;
	object conn;
	string id;

	user = this_object();

	while (user <- LIB_CONN) {
		user = user->query_user();
	}

	if (id = user->query_username()) {
		return id;
	}

	conn = this_object()->query_conn();

	while (conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	if (conn) {
		return query_ip_number(conn);
	} else {
		return "(linkdead)";
	}
}

static void log_message_in(string source, string msg, varargs string key)
{
	mixed *mtime;

	mtime = millitime();

	"~/sys/logd"->post_message("log/log-" + source + "-in",
		"[" + SUBD->pmtime(mtime) + "] " + (key ? key  + " " : "") + "<<< " + msg);
	"~/sys/logd"->post_message("log/log-" + source + "-combo",
		"[" + SUBD->pmtime(mtime) + "] " + (key ? key  + " " : "") + "<<< " + msg);
}

static void log_message_out(string source, string msg, varargs string key)
{
	mixed *mtime;

	mtime = millitime();

	"~/sys/logd"->post_message("log/log-" + source + "-out",
		"[" + SUBD->pmtime(mtime) + "] " + (key ? key  + " " : "") + ">>> " + msg);
	"~/sys/logd"->post_message("log/log-" + source + "-combo",
		"[" + SUBD->pmtime(mtime) + "] " + (key ? key  + " " : "") + ">>> " + msg);
}
