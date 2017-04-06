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

#define POP_SET		0
#define POP_ADD		1
#define POP_AND		2
#define POP_DIV		3
#define POP_MUL		4
#define POP_OR		5
#define POP_RADD	6
#define POP_SUB		7
#define POP_XOR		8

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

private void do_pop(string input, int op)
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

	value = PARSER_VALUE->parse(pvalue);

	if (op == POP_SET) {
		obj->set_property(pname, value);
	} else {
		mixed ovalue;

		ovalue = obj->query_property(pname);

		switch(op) {
		case POP_ADD:
			value = ovalue + value;
			break;

		case POP_AND:
			value = ovalue & value;
			break;

		case POP_DIV:
			value = ovalue / value;
			break;

		case POP_MUL:
			value = ovalue * value;
			break;

		case POP_OR:
			value = ovalue | value;
			break;

		case POP_RADD:
			value = value + ovalue;
			break;

		case POP_SUB:
			value = ovalue - value;
			break;

		case POP_XOR:
			value = ovalue ^ value;
			break;
		}

		obj->set_property(pname, value);
	}
}

private void do_input(string input)
{
	string args;
	string first;

	if (!sscanf(input, "%s %s", first, args)) {
		first = input;
		args = "";
	}

	switch(first) {
	case "":
		break;

	case "quit":
		stopped = 1;
		pop_state();
		return;

	case "pset":
		do_pop(args, POP_SET);
		break;

	case "padd":
		do_pop(args, POP_ADD);
		break;

	case "pand":
		do_pop(args, POP_AND);
		break;

	case "pdiv":
		do_pop(args, POP_DIV);
		break;

	case "pmul":
		do_pop(args, POP_MUL);
		break;

	case "por":
		do_pop(args, POP_OR);
		break;

	case "pradd":
		do_pop(args, POP_RADD);
		break;

	case "psub":
		do_pop(args, POP_SUB);
		break;

	case "pxor":
		do_pop(args, POP_XOR);
		break;

	case "pget":
		if (args != "") {
			send_out(STRINGD->mixed_sprint(obj->query_property(args)) + "\n");
		} else {
			send_out("What property?\n");
		}
		break;

	case "walk":
	case "look":
		{
			object verb;

			verb = VERBD->find_verb(first);

			if (!verb) {
				send_out("No such command.\n");
				break;
			}

			"~/sys/englishd"->do_verb(verb, first, args);
			break;
		}

	case "plist":
		{
			string *props;

			props = obj->list_local_properties();

			send_out(STRINGD->wordwrap(implode(props, ", "), 60) + "\n");
		}

		break;

	default:
		send_out("No such command.\n");
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

void end()
{
	destruct_object(this_object());
}
