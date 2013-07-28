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
#include <kotaka/paths.h>
#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>
#include <thing/paths.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	object user;

	user = query_user();

	if (user->query_class() < 1) {
		send_out("You do not have sufficient access rights to toggle invisibility.\n");
		return;
	}

	switch(args)
	{
	case "on":
		ACCOUNTD->set_account_property(user->query_username(), "invisible", 1);
		break;

	case "off":
		ACCOUNTD->set_account_property(user->query_username(), "invisible", nil);
		break;

	default:
		if (ACCOUNTD->query_account_property(user->query_username(), "invisible")) {
			send_out("You are invisible.\n");
		} else {
			send_out("You are not invisible.\n");
		}
	}
}
