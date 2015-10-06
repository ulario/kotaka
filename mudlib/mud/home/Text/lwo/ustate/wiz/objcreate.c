/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/string.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <kotaka/privilege.h>

#define STAGE_NAME	0
#define STAGE_BRIEF	1
#define STAGE_LOOK	2
#define STAGE_SNOUN	3
#define STAGE_PNOUN	4
#define STAGE_ADJ	5

inherit TEXT_LIB_USTATE;

int reading;
int stopped;
int dead;
int stage;

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
	mixed data;

	switch(stage) {
	case STAGE_NAME:
		data = obj->query_object_name();
		if (data) {
			send_out("Object name: " + data + "\n");
		}
		send_out("Please name this object: ");
		break;

	case STAGE_BRIEF:
		data = obj->query_local_property("brief");
		if (data) {
			send_out("Current brief description: " + data + "\n");
		}
		send_out("Please give a brief (- to reset): ");
		break;

	case STAGE_LOOK:
		data = obj->query_local_property("look");
		if (data) {
			send_out("Current look description:\n" + data + "\n");
		}
		send_out("Please give a look (- to reset): ");
		break;

	case STAGE_SNOUN:
		data = obj->query_local_property("local_snouns");
		if (data && sizeof(data)) {
			send_out("Current singular nouns: " + implode(data, ", ") + "\n");
		}
		send_out("Please give a singular noun (blank to stop, - to reset): ");
		break;

	case STAGE_PNOUN:
		data = obj->query_local_property("local_pnouns");
		if (data && sizeof(data)) {
			send_out("Current plural nouns: " + implode(data, ", ") + "\n");
		}
		send_out("Please give a plural noun (blank to stop, - to reset): ");
		break;

	case STAGE_ADJ:
		data = obj->query_local_property("local_adjectives");
		if (data && sizeof(data)) {
			send_out("Current adjectives: " + implode(data, ", ") + "\n");
		}
		send_out("Please give an adjective (blank to stop, - to reset): ");
		break;
	}
}

void set_object(object o)
{
	ACCESS_CHECK(INTERFACE());

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

private void handle_word(string word, string property, int remove)
{
	string *words;

	words = obj->query_local_property(property);

	if (!words) {
		words = ({ });
	}

	if (remove) {
		words -= ({ word });
	} else {
		words |= ({ word });
	}

	if (!sizeof(words)) {
		words = nil;
	}

	obj->set_local_property(property, words);
}

private void do_input(string input)
{
	switch(stage) {
	case STAGE_NAME:
		if (input == "-") {
			obj->set_object_name(nil);
		} else if (input != "") {
			obj->set_object_name(input);
		}
		stage = STAGE_BRIEF;
		break;

	case STAGE_BRIEF:
		if (input == "-") {
			obj->set_local_property("brief", nil);
		} else if (input != "") {
			obj->set_local_property("brief", input);
		}
		stage = STAGE_LOOK;
		break;

	case STAGE_LOOK:
		if (input == "-") {
			obj->set_local_property("look", nil);
		} else if (input != "") {
			obj->set_local_property("look", input);
		}
		stage = STAGE_SNOUN;
		break;

	case STAGE_SNOUN:
		if (input == "") {
			stage = STAGE_PNOUN;
		} else {
			if (input[0] == '-') {
				handle_word(input[1 ..], "local_snouns", 1);
			} else {
				handle_word(input, "local_snouns", 0);
			}
		}
		break;

	case STAGE_PNOUN:
		if (input == "") {
			stage = STAGE_ADJ;
		} else {
			if (input[0] == '-') {
				handle_word(input[1 ..], "local_pnouns", 1);
			} else {
				handle_word(input, "local_pnouns", 0);
			}
		}
		break;

	case STAGE_ADJ:
		if (input == "") {
			stopped = 1;
			pop_state();
		} else {
			if (input[0] == '-') {
				handle_word(input[1 ..], "local_adjectives", 1);
			} else {
				handle_word(input, "local_adjectives", 0);
			}
		}
		break;
	}
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	if (!obj) {
		send_out("The object you were configuring no longer exists.\n");
		pop_state();
		return;
	}

	reading = 1;

	catch {
		do_input(input);
	} : {
		send_out("Error.\n");
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}
