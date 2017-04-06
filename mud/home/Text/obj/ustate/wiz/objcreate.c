/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2015  Raymond Jennings
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
#define STAGE_ID	1
#define STAGE_BRIEF	2
#define STAGE_PROPER	3
#define STAGE_DEFINITE	4
#define STAGE_LOOK	5
#define STAGE_SNOUN	6
#define STAGE_PNOUN	7
#define STAGE_ADJ	8

#define STAGE_VIRTUAL	9
#define STAGE_MASS	10
#define STAGE_DENSITY	11
#define STAGE_CAPACITY	12
#define STAGE_MAX_MASS	13

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
		send_out("Please give this object a global name if you want (- to reset): ");
		break;

	case STAGE_ID:
		data = obj->query_id();
		if (data) {
			send_out("Object id: " + data + "\n");
		}
		send_out("Please give this object a local id if you want (- to reset): ");
		break;

	case STAGE_BRIEF:
		data = obj->query_local_property("brief");
		if (data) {
			send_out("Current brief description: " + data + "\n");
		}
		send_out("Please give a brief (- to reset): ");
		break;

	case STAGE_PROPER:
		data = obj->query_local_property("is_proper");
		if (data) {
			send_out("Object brief is currently proper.\n");
		} else {
			send_out("Object brief is currently not proper.\n");
		}
		send_out("Should this object's brief be proper? ");
		break;

	case STAGE_DEFINITE:
		data = obj->query_local_property("is_definite");
		if (data) {
			send_out("Object brief is currently definite.\n");
		} else {
			send_out("Object brief is currently not definite.\n");
		}
		send_out("Should this object's brief be definite? ");
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

	case STAGE_VIRTUAL:
		data = obj->query_virtual();
		if (data) {
			send_out("Object is currently virtual.\n");
		} else {
			send_out("Object is currently not virtual.\n");
		}
		send_out("Should this object be virtual? ");
		break;

	case STAGE_MASS:
		data = obj->query_mass();

		send_out("Object's current mass: " + data + " kg.\n");
		send_out("How massive should this object be? ");
		break;

	case STAGE_DENSITY:
		data = obj->query_density();

		send_out("Object's current mass: " + data + " kg/l.\n");
		send_out("How dense should this object be? ");
		break;

	case STAGE_CAPACITY:
		data = obj->query_capacity();

		send_out("Object's current capacity: " + data + " m^3.\n");
		send_out("How much in volume should this object be able to hold? ");
		break;

	case STAGE_MAX_MASS:
		data = obj->query_max_mass();

		send_out("Object's current max mass: " + data + " m^3.\n");
		send_out("How much in mass should this object be able to hold? ");
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
		stage = STAGE_ID;
		break;

	case STAGE_ID:
		if (input == "-") {
			obj->set_id(nil);
		} else if (input != "") {
			obj->set_id(input);
		}
		stage = STAGE_BRIEF;
		break;

	case STAGE_BRIEF:
		if (input == "-") {
			obj->set_local_property("brief", nil);
		} else if (input != "") {
			obj->set_local_property("brief", input);
		}
		stage = STAGE_PROPER;
		break;

	case STAGE_PROPER:
		switch(STRINGD->to_lower(input)) {
		case "":
			if (obj->query_local_property("is_proper")) {
				obj->set_local_property("is_definite", nil);
				stage = STAGE_LOOK;
			} else {
				stage = STAGE_DEFINITE;
			}
			break;

		case "y":
			obj->set_local_property("is_proper", 1);
			obj->set_local_property("is_definite", nil);
			stage = STAGE_LOOK;
			break;

		case "n":
			obj->set_local_property("is_proper", nil);
			stage = STAGE_DEFINITE;
			break;

		default:
			send_out("Yes, no, or blank to leave it alone.\n");
		}
		break;

	case STAGE_DEFINITE:
		switch(STRINGD->to_lower(input)) {
		case "":
			stage = STAGE_LOOK;
			break;

		case "y":
			obj->set_local_property("is_definite", 1);
			stage = STAGE_LOOK;
			break;

		case "n":
			obj->set_local_property("is_definite", nil);
			stage = STAGE_LOOK;
			break;

		default:
			send_out("Yes, no, or blank to leave it alone.\n");
			break;
		}
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
			stage = STAGE_VIRTUAL;
		} else {
			if (input[0] == '-') {
				handle_word(input[1 ..], "local_adjectives", 1);
			} else {
				handle_word(input, "local_adjectives", 0);
			}
		}
		break;

	case STAGE_VIRTUAL:
		switch(STRINGD->to_lower(input)) {
		case "":
			if (obj->query_virtual()) {
				pop_state();
				stopped = 1;
			} else {
				stage = STAGE_MASS;
			}
			break;

		case "y":
			obj->set_mass(0.0);
			obj->set_density(1.0);
			obj->set_capacity(0.0);
			obj->set_max_mass(0.0);
			obj->set_virtual(1);
			pop_state();
			stopped = 1;
			break;

		case "n":
			obj->set_virtual(0);
			stage = STAGE_MASS;
			break;

		default:
			send_out("Yes, no, or blank to leave it alone.\n");
		}
		break;

	case STAGE_MASS:
		if (input == "") {
			stage = STAGE_DENSITY;
		} else {
			float mass;

			if (!sscanf(input, "%f", mass)) {
				send_out("Not a floating point number.\n");
			}

			obj->set_mass(mass);
			stage = STAGE_DENSITY;
		}
		break;

	case STAGE_DENSITY:
		if (input == "") {
			stage = STAGE_CAPACITY;
		} else {
			float density;

			if (!sscanf(input, "%f", density)) {
				send_out("Not a floating point number.\n");
			}

			obj->set_density(density);
			stage = STAGE_CAPACITY;
		}
		break;

	case STAGE_CAPACITY:
		if (input == "") {
			stage = STAGE_MAX_MASS;
		} else {
			float capacity;

			if (!sscanf(input, "%f", capacity)) {
				send_out("Not a floating point number.\n");
			}

			obj->set_capacity(capacity);
			stage = STAGE_MAX_MASS;
		}
		break;

	case STAGE_MAX_MASS:
		if (input == "") {
			pop_state();
			stopped = 1;
		} else {
			float max_mass;

			if (!sscanf(input, "%f", max_mass)) {
				send_out("Not a floating point number.\n");
			}

			obj->set_max_mass(max_mass);
			pop_state();
			stopped = 1;
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
