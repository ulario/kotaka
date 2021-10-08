/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020, 2021  Raymond Jennings
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
#include <kernel/user.h>
#include <kotaka/assert.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

private void usage()
{
	send_out("Usage: kernel <command> <parameters>\n\n");
	send_out("Commands:\n");
	send_out("owners    List resource owners\n");
	send_out("addowner  Add a resource owner\n");
	send_out("rmowner   Remove a resource owner\n");
	send_out("users     List users\n");
	send_out("adduser   Add a user\n");
	send_out("rmuser    Remove a user\n");
	send_out("reset     Reset ACCESSD data\n");
	send_out("save      Save ACCESSD data\n");
	send_out("restore   Restore ACCESSD data\n");
}

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Kernel";
}

string *query_help_contents()
{
	return ({ "Provides administrative access to the kernel library for actions not available from the standard wiztool" });
}

void main(object actor, mapping roles)
{
	object proxy, user;
	string args, command;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can manage the kernel library.\n");
		return;
	}

	args = roles["raw"];

	if (!args) {
		usage();
		return;
	}

	if (!sscanf(args, "%s %s", command, args)) {
		command = args;
		args = nil;
	}

	proxy = PROXYD->get_proxy(query_user()->query_name());

	switch(command) {
	case "help":
		usage();
		break;

	case "owners":
		if (args) {
			send_out("Usage: kernel owners\n");
		} else {
			string *owners;

			owners = KERNELD->query_owners();

			if (sizeof(owners & ({ nil }) ) ) {
				owners -= ({ nil });
				owners = ({ "Ecru" }) + owners;
			}

			send_out(implode(owners, ", ") + "\n");
		}
		break;

	case "users":
		if (args) {
			send_out("Usage: kernel users\n");
		} else {
			string *users;

			users = KERNELD->query_users();

			send_out(implode(users, ", ") + "\n");
		}
		break;

	case "addowner":
		if (!args) {
			send_out("Usage: kernel addowner <owner>\n");
		} else {
			proxy->add_owner(args);
		}
		break;

	case "rmowner":
		if (!args) {
			send_out("Usage: kernel rmowner <owner>\n");
		} else {
			if (args == "admin") {
				send_out("You cannot remove the admin owner.\n");
				return;
			}

			proxy->remove_owner(args);
		}
		break;

	case "adduser":
		if (!args) {
			send_out("Usage: kernel adduser <user>\n");
		} else {
			proxy->add_user(args);
		}
		break;

	case "rmuser":
		if (!args) {
			send_out("Usage: kernel rmuser <user>\n");
		} else {
			if (args == "admin") {
				send_out("You cannot remove the admin user.\n");
				return;
			}

			proxy->remove_user(args);
		}
		break;

	case "reset":
		if (args) {
			send_out("Usage: kernel reset\n");
		} else {
			KERNELD->reset_accessd();
			send_out("AccessD reset.\n");
		}
		return;

	case "save":
		if (args) {
			send_out("Usage: kernel save\n");
		} else {
			KERNELD->save_accessd();
			send_out("AccessD saved.\n");
		}
		return;

	case "restore":
		if (args != nil) {
			send_out("Usage: kernel restore\n");
		} else {
			KERNELD->restore_accessd();
			send_out("AccessD restored.\n");
		}
		return;

	default:
		send_out("Not a valid kernel subcommand.\n");
	}
}
