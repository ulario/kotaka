/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/verb.h>
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

		send_out("[" + query_user()->query_titled_name() + "@ulario] ");
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
		TEXT_SUBD->send_logout_message(user->query_username());
	}
	send_out("Come back soon.\n");
}

private void handle_input(string input, varargs mapping dup);

atomic private void do_atomic(string args)
{
	handle_input(args);
}

private void do_plus_command(string input)
{
	string first;

	if (!first && !sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	switch(first) {
	case "help":
		send_out("+ commands:\n");
		send_out("atomic   Execute the command atomically.\n");
		send_out("fixverbs Reboot the verb module, and reload all verbs.\n");
		return;

	case "fixverbs":
		MODULED->reboot_module("Verb");
		return;

	case "atomic":
		do_atomic(input);
		return;

	default:
		send_out("Invalid plus command.\n");
		return;
	}
}

private void handle_input(string input, varargs mapping dup)
{
	string first;
	string alias;

	input = STRINGD->trim_whitespace(input);

	if (strlen(input) > 0) {
		switch(input[0]) {
		case '%':
			if (query_user()->query_class() >= 2) {
				query_user()->dispatch_wiztool(input[1 ..]);
			} else {
				send_out("Only game staff are allowed to use the wiztool.\n");
			}
			return;

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

		case '+':
			do_plus_command(input[1 ..]);
			return;
		}
	}

	if (!first && !sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	if (first == "") {
		return;
	}

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

	catch {
		object verb;

		verb = VERBD->find_verb(first);

		if (!verb) {
			send_out("No such command.\n");
			return;
		}

		"~/sys/englishd"->do_verb(verb, first, input);
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
