/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020, 2021  Raymond Jennings
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
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Chrestore";
}

string *query_help_contents()
{
	return ({ "Restores channel configuration" });
}

void main(object actor, mapping roles)
{
	object user;
	string name, *subscriptions;

	user = query_user();
	name = user->query_username();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to retore channels.\n");
		return;
	}

	if (roles["raw"]) {
		send_out("Usage: chrestore\n");
		return;
	}

	CHANNELD->restore();
	send_out("Channels restored.\n");
}
