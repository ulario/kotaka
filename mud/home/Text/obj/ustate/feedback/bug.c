/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2022  Raymond Jennings
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
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>

inherit LIB_USTATE;

string topic;
string body;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	send_out("Topic: ");
}

void receive_in(string str)
{
	ACCESS_CHECK(previous_object() == query_user());

	if (!topic) {
		topic = str;
		body = "";

		send_out("Please fill out details.\n\nTry to include what you were doing, where you were, etc.\n\nEnd with a blank line.\n\n");
	} else {
		if (str == "") {
			write_file("~/data/bugs", "From: " + query_user()->query_name() + "\nTime:" + ctime(time()) + "\nTitle: " + topic + "\n\n" + body + "\n");
			send_out("Thanks for the bug report, it's been put into the queue.\n");
			pop_state();
			return;
		}

		body += str + "\n";
	}
}

void end()
{
	destruct_object(this_object());
}
