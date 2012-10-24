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
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <text/paths.h>

inherit LIB_USTATE;

int stopped;
int reading;
int introed;

object wiztool;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

private void prompt()
{
	string str;

	if (str = query_editor(wiztool)) {
		if (str == "insert") {
			send_out(": ");
		} else {
			send_out("Edit> ");
		}
	} else {
		send_out("[\033[1;30mWiztool\033[0m] ");
	}
}

void message(string str)
{
	ACCESS_CHECK(previous_object() == wiztool);

	send_out(str);
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	wiztool = WIZTOOLD->get_wiztool();

	send_out("Emergency wiztool activated.\n\n");
}

void stop()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 1;
}

void go()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 0;

	if (!reading) {
		prompt();
	}
}

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	wiztool->dispose();

	destruct_object(this_object());
}

void receive_in(string input)
{
	string first;

	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (!sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	switch(first) {
	case "":
		break;
	case "quit":
		pop_state();
		return;
	default:
		wiztool->input(first + " " + input);
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}
