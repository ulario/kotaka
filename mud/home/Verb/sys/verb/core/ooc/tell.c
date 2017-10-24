/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kotaka/paths/channel.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	object user;
	mixed tuser;
	string channel;
	string text;

	user = query_user();

	if (user->query_class() < 1) {
		send_out("You do not have sufficient access rights to send tells.\n");
		return;
	}

	if (sscanf(roles["raw"], "%s %s", tuser, text) != 2) {
		send_out("Usage: tell <user> <text>\n");
		return;
	}

	tuser = TEXT_USERD->find_user(tuser);

	if (!tuser) {
		send_out("User not found.\n");
		return;
	}

	tuser->message(user->query_titled_name() + " tells you: " + text + "\n");
	user->message("You tell " + tuser->query_titled_name() + ": " + text + "\n");
}
