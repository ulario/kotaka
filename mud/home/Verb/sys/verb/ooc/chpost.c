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
#include <kotaka/paths/channel.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Chpost";
}

string *query_help_contents()
{
	return ({ "Posts a channel." });
}

void main(object actor, mapping roles)
{
	object user;
	string args, channel, text;

	user = query_user();

	if (user->query_class() < 1) {
		send_out("You do not have sufficient access rights to post to generic channels.\n");
		return;
	}

	args = roles["raw"];

	if (!args || !sscanf(args, "%s %s", channel, text)) {
		send_out("Usage: chpost <channel> <message>\n");
		return;
	}

	if (CHANNELD->query_intermud(channel) && user->query_class() < 2) {
		send_out("Access to intermud channels restricted to wizards.\n");
		return;
	}

	if (!CHANNELD->test_channel(channel)) {
		send_out("No such channel.\n");
		return;
	}

	CHANNELD->post_message(channel, user->query_name(), text);
}
