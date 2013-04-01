/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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
#include <kernel/access.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <text/paths.h>

inherit TEXT_LIB_USTATE;

int stopped;
int reading;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

/* int do_action(string cmd, object actor, string args) */

private void prompt()
{
	object body;

	body = query_user()->query_body();

	if (body) {
		send_out(body->query_property("id") + "> ");
	} else {
		string name;

		name = query_user()->query_username();

		if (!name) {
			name = "guest";
		}

		send_out("[" + name + "@ulario] ");
	}
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
	object user;
	string name;

	user = query_user();

	ACCESS_CHECK(previous_object() == user);

	if (user->query_username()) {
		TEXT_SUBD->send_to_all_except(
			TEXT_SUBD->titled_name(
				user->query_username(),
				user->query_class())
			+ " logs out.\n", ({ user }));
	}
	send_out("Come back soon.\n");
}

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	destruct_object(this_object());
}

private int is_wiztool_command(string command)
{
	switch(command) {
	case "code":
	case "compile":
	case "clone":
	case "destruct":

	case "ls":
	case "cp":
	case "mv":
	case "rm":
	case "mkdir":
	case "rmdir":

	case "access":
	case "grant":
	case "ungrant":
	case "quota":
	case "rsrc":

	case "people":
	case "status":
	case "swapout":
	case "statedump":
	case "shutdown":
	case "reboot":
		return 1;
	default:
		return 0;
	}
}

private void do_input(string first, string input)
{
	object body;

	body = query_user()->query_body();

	if (VERBD->do_action(body, first, input)) {
		return;
	}

	if (query_user()->query_class() >= 2) {
		if (is_wiztool_command(first)) {
			query_user()->dispatch_wiztool(first + " " + input);
			return;
		}
	}

	if (body) {
		send_out("No such command or verb.\n");
	} else {
		send_out("No such command.\n");
	}
}

void receive_in(string input)
{
	string first;

	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (strlen(input) > 0) {
		switch(input[0]) {
		case '\'':
			input = input[1 ..];
			first = "say";
			break;
		case ':':
			input = input[1 ..];
			first = "emote";
			break;
		}
	}

	if (!first && !sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	switch(first) {
	case "":
		break;
	default:
		do_input(first, input);
	}

	if (!this_object()) {
		return;
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}
