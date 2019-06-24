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
#include <kernel/kernel.h>
#include <kernel/user.h>

private string whoami()
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

	while (conn && conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	if (conn) {
		return query_ip_number(conn);
	} else {
		return "(linkdead)";
	}
}

static void log_message(string message)
{
	"~/sys/logd"->log_message(whoami(), message);
}
