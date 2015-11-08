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
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

private int check_module_name(string module)
{
	if (!STRINGD->is_alpha(module)) {
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
	send_out("boot     - to boot a module.\n");
	send_out("reboot   - to reboot a module.\n");
	send_out("shutdown - to shutdown a module.\n");
}

void main(object actor, mapping roles)
{
	object user;
	string name;
	string command;
	string module;

	user = query_user();
	name = user->query_username();
	command = roles["raw"];

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to manage modules.\n");
		return;
	}

	if (sscanf(command, "%s %s", command, module) < 2) {
		usage();
		return;
	}

	switch(command) {
	case "boot":
		if (!check_module_name(module)) {
			return;
		}
		MODULED->boot_module(module);
		break;
	case "reboot":
		if (!check_module_name(module)) {
			return;
		}
		MODULED->reboot_module(module);
		break;
	case "shutdown":
		if (!check_module_name(module)) {
			return;
		}
		MODULED->shutdown_module(module);
		break;
	default:
		usage();
	}
}
