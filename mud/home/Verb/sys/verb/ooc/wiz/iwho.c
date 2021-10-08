/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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
#include <kotaka/paths/intermud.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Iwho";
}

string *query_help_contents()
{
	return ({ "Queries another mud on I3 for a list of their online players" });
}

void main(object actor, mapping roles)
{
	object user;
	string msg, mud, target;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You have insufficient access to send i3 whos.\n");
		return;
	}

	mud = roles["raw"];

	if (!mud) {
		send_out("Usage: iwho <mud name>\n");
		return;
	}

	INTERMUDD->send_who(user->query_name(), mud);

	user->message("You iwho " + mud + "\n");
}
