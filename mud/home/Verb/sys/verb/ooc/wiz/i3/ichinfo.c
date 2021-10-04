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
#include <kotaka/paths/verb.h>

inherit "/lib/string/sprint";
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "ichinfo";
}

string *query_help_contents()
{
	return ({ "Usage: ichinfo <channel>", "Returns information about a channel on I3" });
}

void main(object actor, mapping roles)
{
	string channel;

	if (query_user()->query_class() < 2) {
		send_out("You have insufficient access to query i3 channel info.\n");
		return;
	}

	channel = roles["raw"];

	if (!channel) {
		send_out("Usage: ichinfo <channel>\n");
		return;
	}

	send_out(hybrid_sprint(INTERMUDD->query_channel(channel)) + "\n");
}
