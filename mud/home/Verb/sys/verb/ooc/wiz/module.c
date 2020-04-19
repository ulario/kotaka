/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>

inherit "/lib/string/validate";
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

private int check_module_name(string module)
{
	if (!is_alpha(module)) {
		send_out("Invalid module name.\n");
		return 0;
	}

	if (module[0] < 'A' || module[0] > 'Z') {
		send_out("Invalid module name.\n");
		return 0;
	}

	return 1;
}

private void usage()
{
	send_out("Usage: module <subcommand> <module name>\n");
	send_out("Subcommands:\n");
	send_out("boot     - boot a module.\n");
	send_out("reboot   - reboot a module.\n");
	send_out("shutdown - shutdown a module.\n");
	send_out("list     - list active modules.\n");
}

void main(object actor, mapping roles)
{
	object user;
	string name;
	string command;
	string args;

	user = query_user();
	name = user->query_username();
	command = roles["raw"];

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to manage modules.\n");
		return;
	}

	sscanf(command, "%s %s", command, args);

	switch(command) {
	case "boot":
		if (!args) {
			usage();
			return;
		}

		if (!check_module_name(args)) {
			return;
		}

		if (args == "Ecru") {
			args = nil;
		}

		MODULED->boot_module(args);
		break;

	case "reboot":
		if (!args) {
			usage();
			return;
		}

		if (!check_module_name(args)) {
			return;
		}

		if (args == "Ecru") {
			args = nil;
		}

		MODULED->reboot_module(args);
		break;

	case "shutdown":
		if (!args) {
			usage();
			return;
		}

		if (!check_module_name(args)) {
			return;
		}

		if (args == "Ecru") {
			args = nil;
		}

		MODULED->shutdown_module(args);
		break;

	case "list":
		{
			string *modules;

			modules = MODULED->query_modules();

			if (sizeof(modules & ({ nil }))) {
				modules -= ({ nil });
				modules = ({ "Ecru" }) | modules;
			}

			send_out("Active modules: " + implode(modules, ", ") + "\n");
		}
		break;

	default:
		usage();
	}
}
