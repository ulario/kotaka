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
#include <kernel/access.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>

inherit GAME_LIB_USTATE;

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

private void prompt()
{
	string name;

	name = query_user()->query_username();

	if (!name) {
		name = "guest";
	}

	send_out("[" + name + "@ulario] ");
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	if (!query_user()->query_username()) {
		send_out(read_file("~/data/doc/guest_welcome"));
	}
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
		GAME_SUBD->send_to_all_except(
			GAME_SUBD->titled_name(
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
		if (BIND->execute_command("adm/" + first, input))
			break;
		if (BIND->execute_command("wiz/tool/" + first, input))
			break;
		if (BIND->execute_command("wiz/debug/" + first, input))
			break;
		if (BIND->execute_command("wiz/" + first, input))
			break;
		if (BIND->execute_command("movie/" + first, input))
			break;
		if (BIND->execute_command(first, input))
			break;
		send_out("No such command.\n");
	}

	if (!this_object()) {
		return;
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}
