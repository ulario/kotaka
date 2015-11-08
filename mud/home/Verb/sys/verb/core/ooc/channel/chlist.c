/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/string.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	object user;

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

	if (sizeof(active)) {
		send_out("Subscribed channels:\n\n");
		send_out(STRINGD->wordwrap(implode(active, ", "), 60));
		send_out("\n\n");
	}

	if (sizeof(inactive)) {
		send_out("Unsubscribed channels:\n\n");
		send_out(STRINGD->wordwrap(implode(inactive, ", "), 60));
		send_out("\n\n");
	}

	if (sizeof(dead)) {
		send_out("Orphaned channels:\n\n");
		send_out(STRINGD->wordwrap(implode(dead, ", "), 60));
		send_out("\n\n");
	}
}
