/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019  Raymond Jennings
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
#include <kotaka/log.h>
#include <kotaka/assert.h>
#include <kernel/user.h>

private string saved_ip;

static void log_set_ip()
{
	catch {
		object conn;

		conn = this_object()->query_conn();

		ASSERT(conn);

		while (conn <- LIB_USER) {
			conn = conn->query_conn();
		}

		saved_ip = query_ip_number(conn);

		ASSERT(saved_ip);
	}
}

static void log_message(string message)
{
	object this, user, conn;

	this = this_object();
	user = this;

	while (user <- LIB_CONN) {
		user = user->query_user();
	}

	if (user) {
		string username;

		username = user->query_username();

		if (username) {
			"~/sys/logd"->log_message(username, message);
			return;
		}
	} else {
		LOGD->post_message("system", LOG_WARNING, "While logging: Object " + object_name(this_object()) + " has no user");
		LOGD->post_message("system", LOG_WARNING, "Log message: " + message);
		return;
	}

	conn = this->query_conn();

	while (conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	if (conn) {
		string ip;

		ip = query_ip_number(conn);

		if (ip) {
			"~/sys/logd"->log_message(ip, message);
			return;
		}

		if (saved_ip) {
			"~/sys/logd"->log_message(saved_ip, message);
		} else {
			LOGD->post_message("system", LOG_WARNING, "While logging: Object " + object_name(this_object()) + " is linkdead and no saved IP");
			LOGD->post_message("system", LOG_WARNING, "Log message: " + message);
		}
	} else {
		LOGD->post_message("system", LOG_WARNING, "While logging: Object " + object_name(this_object()) + " has no connection");
		LOGD->post_message("system", LOG_WARNING, "Log message: " + message);
	}
}
