/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/paths.h>
#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	string name;
	string gender;

	name = query_user()->query_name();

	if (!name) {
		send_out("You aren't logged in.\n");
		return;
	}

	if (args == "") {
		switch(ACCOUNTD->query_account_property(name, "gender")) {
		case nil:
			send_out("You have no gender.\n");
			break;

		case "male":
			send_out("You are male.\n");
			break;

		case "female":
			send_out("You are female.\n");
			break;
		}
	} else {
		switch(args) {
		case "male":
			ACCOUNTD->set_account_property(name, "gender", "male");
			send_out("You are now male.\n");
			break;

		case "female":
			ACCOUNTD->set_account_property(name, "gender", "female");
			send_out("You are now female.\n");
			break;

		case "none":
			ACCOUNTD->set_account_property(name, "gender", nil);
			send_out("You are now neutral.\n");
			break;

		default:
			send_out("Male, female, or none please.\n");
		}
	}
}
