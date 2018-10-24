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
#include <kotaka/paths/ansi.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>
#include <kotaka/telnet.h>

inherit LIB_FILTER;

string *lines;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

int login(string str)
{
	return ::login(str);
}

int receive_message(string str)
{
	if (!lines) {
		lines = ({ });

		call_out("trickle", 0);
	}

	lines += ({ str });

	return MODE_NOCHANGE;
}

static void trickle()
{
	string line;
	object this_user;

	if (!lines || !sizeof(lines)) {
		return;
	}

	line = lines[0];
	lines = lines[1 ..];

	if (sizeof(lines)) {
		call_out("trickle", 0);
	} else {
		lines = nil;
	}

	this_user = SYSTEM_USERD->query_this_user();

	catch {
		object user;

		user = this_object();

		while (user && user <- LIB_CONN) {
			user = user->query_user();
		}

		SYSTEM_USERD->set_this_user(user);

		::receive_message(line);
	}

	SYSTEM_USERD->set_this_user(this_user());
}
