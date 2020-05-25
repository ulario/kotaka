/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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

inherit "/lib/string/format";
inherit "/lib/string/sprint";
inherit TEXT_LIB_USTATE;

int reading;
int stopped;
int dead;

object obj;
string detail;

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
	if (detail) {
		send_out("[\033[1;34mobject configuration (" + detail + ")\033[0m] ");
	} else {
		send_out("[\033[1;34mobject configuration\033[0m] ");
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

private void do_help()
{
	send_out("Commands\n");
	send_out("--------\n");
	send_out("help, quit, pset, padd, pand, pdiv, pul, por, pradd, psub, pxor, pget\n");
	send_out("walk, look\n");
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

	case "help":
		do_help();
		break;

	case "quit":
		stopped = 1;
		pop_state();
		return;

	/* properties */
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
			send_out(mixed_sprint(obj->query_property(args)) + "\n");
		} else {
			send_out("What property?\n");
		}
		break;

	case "lpget":
		if (args != "") {
			send_out(mixed_sprint(obj->query_local_property(args)) + "\n");
		} else {
			send_out("What property?\n");
		}
		break;

	case "lpset":
		if (args != "") {
			string pname, pvalue;
			mixed value;

			if (!sscanf(args, "%s %s", pname, pvalue)) {
				send_out("Property value?\n");
				return;
			}

			value = PARSER_VALUE->parse(pvalue);

			obj->set_local_property(pname, value);
		} else {
			send_out("What property?\n");
		}
		break;

	case "lplist":
		{
			string *props;

			props = obj->list_local_properties();

			send_out(wordwrap(implode(props, ", "), 60) + "\n");
		}
		break;

	/* details */
	case "dadd":
		if (args == "") {
			args = nil;
		}

		obj->add_detail(args);

		if (args) {
			send_out("Detail \"" + args + "\" added, configuring it now.\n");
		} else {
			send_out("Default detail added, configuring it now.\n");
		}

		detail = args;
		break;

	case "ddel":
		{
			if (args == "") {
				obj->remove_detail(nil);
				obj->remove_detail("");
				send_out("Default detail \"" + args + "\" removed.\n");
			} else {
				obj->remove_detail(args);
				send_out("Detail \"" + args + "\" removed.\n");
			}
		}
		break;

	case "dveto":
		if (args == "") {
			obj->veto_detail(nil);
			send_out("Default detail \"" + args + "\" vetoed.\n");
		} else {
			obj->veto_detail(args);
			send_out("Detail \"" + args + "\" vetoed.\n");
		}
		break;

	case "dunveto":
		if (args == "") {
			obj->unveto_detail(nil);
			send_out("Default detail \"" + args + "\" unvetoed.\n");
		} else {
			obj->unveto_detail(args);
			send_out("Detail \"" + args + "\" unvetoed.\n");
		}
		break;

	case "dselect":
		if (args == "") {
			detail = nil;
		} else {
			detail = args;
		}
		break;

	case "combine":
		if (strlen(args)) {
			obj->set_combine(detail, args);
		} else {
			send_out("Usage: combine <combine>\n");
			send_out("overlap - use local values, and inherit from archetype\n");
			send_out("replace - ignore detail from archetype, use only local values\n");
		}
		break;

	case "dlist":
		{
			string *details;

			details = obj->query_details();

			if (sizeof(details & ({ nil }))) {
				details = ({ "(default)" }) + (details - ({ nil }));
			}

			if (sizeof(details)) {
				send_out("Details: " + implode(details, ", ") + "\n");
			} else {
				send_out("No details.\n");
			}
		}
		break;

	case "dview":
		{
			string dname;
			string *descriptions;
			int sz;
			string combine;

			dname = strlen(args) ? args : nil;

			if (!obj->has_detail(dname)) {
				send_out("No such detail.\n");
				break;
			}

			send_out((dname ? "Detail " + dname : "(default detail)") + "\n");

			combine = obj->query_combine(dname);

			if (combine) {
				send_out("Combine policy: " + combine + "\n");
			} else {
				send_out("Default combine policy\n");
			}

			send_out("Singular nouns: " + implode(obj->query_snouns(dname), ", ") + "\n");
			send_out("Plural nouns: " + implode(obj->query_pnouns(dname), ", ") + "\n");
			send_out("Adjectives: " + implode(obj->query_adjectives(dname), ", ") + "\n");

			descriptions = obj->query_descriptions(dname);

			if (sz = sizeof(descriptions)) {
				int i;

				for (i = 0; i < sz; i++) {
					string description;

					description = descriptions[i];
					send_out(description + ": " + obj->query_description(dname, description) + "\n");
				}
			}
		}
		break;

	case "snoun":
		if (strlen(args)) {
			if (args[0] == '-') {
				obj->remove_snoun(detail, args[1 ..]);
			} else {
				obj->add_snoun(detail, args);
			}
		} else {
			send_out("Usage: snoun [-]noun\n");
		}
		break;

	case "pnoun":
		if (strlen(args)) {
			if (args[0] == '-') {
				obj->remove_pnoun(detail, args[1 ..]);
			} else {
				obj->add_pnoun(detail, args);
			}
		} else {
			send_out("Usage: pnoun [-]noun\n");
		}
		break;

	case "adjective":
		if (strlen(args)) {
			if (args[0] == '-') {
				obj->remove_pnoun(detail, args[1 ..]);
			} else {
				obj->add_pnoun(detail, args);
			}
		} else {
			send_out("Usage: pnoun [-]noun\n");
		}
		break;

	case "brief":
		if (strlen(args)) {
			obj->set_description(detail, "brief", args);
		} else {
			obj->set_description(detail, "brief", nil);
		}
		break;

	case "look":
		if (strlen(args)) {
			obj->set_description(detail, "look", args);
		} else {
			obj->set_description(detail, "look", nil);
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
