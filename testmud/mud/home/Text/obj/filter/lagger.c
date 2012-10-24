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

inherit LIB_FILTER;
string outbuf;
int callout;
int escape;
int dead;

private void schedule()
{
	if (callout == -1 && strlen(outbuf)) {
		switch(outbuf[0]) {
		case '\033': escape = 1; break;
		case 'A' .. 'Z': escape = 0; break;
		case 'a' .. 'z': escape = 0; break;
		}

		if (escape) {
			callout = call_out("char", 0);
		} else {
			callout = call_out("char", 0.05);
		}
	}
}

static void create(int clone)
{
	if (clone) {
		outbuf = "";
		callout = -1;
		::create();
	}
}

int message(string str)
{
	ACCESS_CHECK(previous_program() == LIB_USER);

	outbuf += str;
	schedule();

	return 1;
}

void disconnect()
{
	if (query_conn()) {
		dead = 1;
		if (!strlen(outbuf)) {
			::disconnect();
		}
	} else {
		::disconnect();
	}
}

int receive_message(string str)
{
	if (!dead) {
		return ::receive_message(str);
	} else {
		return MODE_NOCHANGE;
	}
}

static void char()
{
	callout = -1;

	::message(outbuf[0 .. 0]);
	outbuf = outbuf[1 ..];

	if (dead && !strlen(outbuf)) {
		::disconnect();
	}

	schedule();
}
