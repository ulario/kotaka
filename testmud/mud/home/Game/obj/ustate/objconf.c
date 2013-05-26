/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <text/paths.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

inherit TEXT_LIB_USTATE;

int reading;
int stopped;
int dead;

object obj;

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
	send_out("[\033[1;34mobject configuration\033[0m] ");
}

void set_object(object o)
{
	ACCESS_CHECK(GAME());

	obj = o;
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());
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

void pre_end()
{
	ACCESS_CHECK(previous_object() == query_user());

	dead = 1;
}

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	destruct_object(this_object());
}

private void do_pset(string input)
{
	string pname;
	string pvalue;
	mixed value;

	if (input == nil) {
		send_out("Property name?\n");
		return;
	}

	if (!sscanf(input, "%s %s", pname, pvalue)) {
		send_out("Property value?\n");
		return;
	}

	obj->set_property(pname, PARSE_VALUE->parse(pvalue));
}

void receive_in(string input)
{
	string args;
	string first;

	ACCESS_CHECK(previous_object() == query_user());

	if (!obj) {
		send_out("The object you were configuring no longer exists.\n");
		pop_state();
		return;
	}

	reading = 1;

	if (!sscanf(input, "%s %s", first, args)) {
		first = input;
	}

	switch(first) {
	case "quit":
		pop_state();
		return;

	case "pset":
		do_pset(args);
		break;

	case "lplist":
		{
			string *props;

			props = obj->list_local_properties();

			send_out(STRINGD->wordwrap(implode(props, ", "), 60) + "\n");
		}

		break;
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}
