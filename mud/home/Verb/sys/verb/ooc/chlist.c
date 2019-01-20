/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kotaka/paths/account.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/verb.h>

inherit "/lib/string/format";
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

private void i3tag(string *chlist)
{
	int sz;

	for (sz = sizeof(chlist); --sz >= 0; ) {
		if (CHANNELD->query_intermud(chlist[sz])) {
			chlist[sz] += " (i3)";
		}
	}
}

string query_help_title()
{
	return "Chlist";
}

string *query_help_contents()
{
	return ({ "Lists channels." });
}

void main(object actor, mapping roles)
{
	object user;
	int sz;

	string *channels;
	string *subscriptions;

	string *active, *inactive, *dead;

	user = query_user();

	if (user->query_class() < 1) {
		send_out("You do not have sufficient access rights to list channels.\n");
		return;
	}

	subscriptions = ACCOUNTD->query_account_property(user->query_name(), "channels");

	if (!subscriptions) {
		subscriptions = ({ });
	}

	channels = CHANNELD->query_channels();

	active = channels & subscriptions;
	inactive = channels - subscriptions;
	dead = subscriptions - channels;

	i3tag(active);
	i3tag(inactive);

	if (sizeof(active)) {
		send_out("Subscribed channels:\n\n");
		send_out(wordwrap(implode(active, ", "), 60));
		send_out("\n\n");
	}

	if (sizeof(inactive)) {
		send_out("Unsubscribed channels:\n\n");
		send_out(wordwrap(implode(inactive, ", "), 60));
		send_out("\n\n");
	}

	if (sizeof(dead)) {
		send_out("Orphaned channels:\n\n");
		send_out(wordwrap(implode(dead, ", "), 60));
		send_out("\n\n");
	}
}
