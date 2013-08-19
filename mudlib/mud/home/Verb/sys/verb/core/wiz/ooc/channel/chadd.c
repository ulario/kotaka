/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/verb.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	object user;
	string name;
	string *subscriptions;

	user = query_user();
	name = user->query_username();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to create channels.\n");
		return;
	}

	if (args == "") {
		send_out("Cat got your tongue?\n");
		return;
	}

	if (CHANNELD->test_channel(args)) {
		send_out("That channel already exists.\n");
		return;
	}

	CHANNELD->add_channel(args);

	send_out("Channel created.\n");
	return;
}
