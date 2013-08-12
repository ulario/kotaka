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
#include <kernel/user.h>
#include <kotaka/assert.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	object user;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to suspend connections.\n");
		return;
	}

	while (user <- LIB_USER) {
		user = user->query_conn();
	}

	ASSERT(user);

	SYSTEM_USERD->block_connections(user);
}
