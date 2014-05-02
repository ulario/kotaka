/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <config.h>
#include <kernel/access.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>

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
		send_out(TEXT_SUBD->generate_brief_indefinite(body) + "> ");
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

private void handle_input(string input, varargs mapping dup);

atomic private void do_atomic(string args)
{
	handle_input(args);
}

private void do_alias(string args)
{
	string cmd;
	string alias;

	if (!sscanf(args, "%s %s", cmd, args)) {
		cmd = args;
		args = nil;
	}

	switch(cmd) {
	case "help":
		send_out("Alias help\n");
		send_out("----------\n");
		send_out("add - Add an alias\n");
		send_out("del - Delete an alias\n");
		send_out("list - List aliases\n");
		send_out("gadd - Add a global alias\n");
		send_out("gdel - Delete a global alias\n");
		send_out("glist - List global aliases\n");
		break;

	case "add":
	case "del":
	case "list":
		send_out("Not yet implemented.\n");
		break;

	case "gadd":
		if (query_user()->query_class() < 3) {
			send_out("Permission denied.\n");
			break;
		}

		if (!sscanf(args, "%s %s", alias, args)) {
			send_out("Please specify the alias and the expansion.\n");
			break;
		}

		ALIASD->set_alias(alias, args);
		send_out("Done.\n");
		break;

	case "gdel":
		if (query_user()->query_class() < 3) {
			send_out("Permission denied.\n");
			break;
		}

		if (sscanf(args, "%s %s", alias, args)) {
			send_out("Don't specify an expansion when removing an alias\n");
			break;
		}

		ALIASD->set_alias(alias, nil);
		send_out("Done.\n");
		break;

	case "glist":
		if (args && sscanf(args, "%s %s", alias, args)) {
			send_out("Don't specify an expansion when listing aliases\n");
			break;
		}

		send_out("Aliases\n");
		send_out("-------\n");

		{
			string *aliases;
			int i, sz;

			aliases = ALIASD->query_aliases();

			sz = sizeof(aliases);

			for (i = 0; i < sz; i++) {
				alias = aliases[i];
				send_out(alias + " - " + ALIASD->query_alias(alias) + "\n");
			}
		}
	}
}

private void handle_input(string input, varargs mapping dup)
{
	string first;
	string alias;

	input = STRINGD->trim_whitespace(input);

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

		case ';':
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
	case "+fixverbs":
		INITD->reboot_subsystem("Verb");
		return;

	case "+atomic":
		do_atomic(input);
		return;

	case "+alias":
		do_alias(input);
		return;

	case "":
		return;

	default:
		if (!dup || !dup[first]) {
			alias = ALIASD->query_alias(first);

			if (alias) {
				if (!dup) {
					dup = ([ ]);
				}

				dup[first] = 1;

				send_out("Expanding to: " + alias + " " + input + "\n");
				handle_input(alias + " " + input, dup);

				dup[first] = nil;

				return;
			}
		}
	}

	if (!first) {
		return;
	}

	if (query_user()->query_class() >= 2) {
		if (is_wiztool_command(first)) {
			query_user()->dispatch_wiztool(first + " " + input);
			return;
		}
	}

	catch {
		if ("~/sys/englishd"->do_verb(first, input)) {
			return;
		} else {
			send_out("No such command.\n");
		}
	} : {
		send_out("Error.\n");
	}
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	catch {
		handle_input(input);
	}

	reading = 0;

	if (!this_object() || !query_user()) {
		return;
	}

	if (!stopped) {
		prompt();
	}
}
