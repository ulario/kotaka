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
#include <kotaka/paths.h>

#include <account/paths.h>
#include <text/paths.h>
#include <game/paths.h>

inherit LIB_VERB;

void main(object actor, mapping roles)
{
	string user;
	int amount;
	mixed total;

	if (query_user()->query_class() < 2) {
		send_out("Only a wizard can award XP.\n");
		return;
	}

	if (sscanf(args, "%s %d", user, amount) != 2) {
		send_out("Usage: award user amount.\n");
		return;
	}

	if (!ACCOUNTD->query_is_registered(user)) {
		send_out("No such user.\n");
		return;
	}

	total = ACCOUNTD->query_account_property(user, "xp");

	if (!total) {
		total = 0;
	}

	total += amount;

	if (total) {
		ACCOUNTD->set_account_property(user, "xp", total);
	} else {
		ACCOUNTD->set_account_property(user, "xp", nil);
	}
}
