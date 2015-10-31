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
#include <kernel/user.h>
#include <kotaka/assert.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string *split_first_word(string input)
{
	string first, second;

	if (sscanf(input, "%s %s", first, second)) {
		return ({ first, second });
	} else {
		return ({ input, nil });
	}
}

void main(object actor, mapping roles)
{
	string command;
	string args;
	object user;
	object proxy;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can manage the kernel library.\n");
		return;
	}

	args = roles["raw"];

	({ command, args }) = split_first_word(args);

	if (!command || command == "") {
		send_out("Usage: kernel <command> <parameters>\n");
		send_out("owners    List resource owners\n");
		send_out("addowner  Add a resource owner\n");
		send_out("rmowner   Remove a resource owner\n");
		send_out("users     List users\n");
		send_out("adduser   Add a user\n");
		send_out("rmuser    Remove a user\n");
		return;
	}

	proxy = PROXYD->get_proxy(query_user()->query_name());

	switch(command) {
	case "owners":
		if (args != nil) {
			send_out("Usage: kernel owners\n");
			return;
		} else {
			string *owners;

			owners = KERNELD->query_owners();

			if (sizeof(owners & ({ nil }) ) ) {
				owners -= ({ nil });
				owners = ({ "Ecru" }) + owners;
			}

			send_out(implode(owners, ", ") + "\n");
			return;
		}

	case "users":
		if (args != nil) {
			send_out("Usage: kernel users\n");
			return;
		} else {
			string *users;

			users = KERNELD->query_users();

			send_out(implode(users, ", ") + "\n");
			return;
		}

	case "addowner":
		if (args == nil) {
			send_out("Usage: kernel addowner <owner>\n");
			return;
		} else {
			proxy->add_owner(args);
		}
		break;

	case "rmowner":
		if (args == nil) {
			send_out("Usage: kernel rmowner <owner>\n");
			return;
		} else {
			if (args == "admin") {
				send_out("You cannot remove the admin owner.\n");
				return;
			}

			proxy->remove_owner(args);
		}
		break;

	case "adduser":
		if (args == nil) {
			send_out("Usage: kernel adduser <user>\n");
			return;
		} else {
			proxy->add_user(args);
		}
		break;

	case "rmuser":
		if (args == nil) {
			send_out("Usage: kernel rmuser <user>\n");
			return;
		} else {
			if (args == "admin") {
				send_out("You cannot remove the admin user.\n");
				return;
			}

			proxy->remove_user(args);
		}
		break;

	default:
		send_out("Not a valid kernel subcommand.\n");
	}
}
