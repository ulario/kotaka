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
#include <kotaka/paths/channel.h>
#include <kotaka/paths/intermud.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "ichon";
}

string *query_help_contents()
{
	return ({ "Usage: ichon <channel>", "Connects a channel to I3" });
}

void main(object actor, mapping roles)
{
	string channel;

	if (query_user()->query_class() < 2) {
		send_out("You have insufficient access to activate i3 channels.\n");
		return;
	}

	channel = roles["raw"];

	if (!channel) {
		send_out("Usage: ichon <channel>\n");
		return;
	}

	if (!CHANNELD->test_channel(channel)) {
		send_out("Channel must exist before it can be attached to i3.\n");
		return;
	}

	CHANNELD->set_intermud(channel, 1);

	send_out("Channel connected to Intermud3.\n");
}
