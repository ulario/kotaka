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
#include <kotaka/paths/account.h>
#include <kotaka/paths/verb.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	string name;
	mixed total;

	name = query_user()->query_name();

	if (!name) {
		send_out("You aren't logged in.\n");
		return;
	}

	total = ACCOUNTD->query_account_property(name, "xp");

	if (total) {
		if (total > 0) {
			send_out("You have " + total + " XP :)\n");
		} else {
			send_out("You are " + -total + " XP in debt! >:(\n");
		}
	} else {
		send_out("You have no XP :(\n");
	}
}
