/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020  Raymond Jennings
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
#include <kernel/access.h>
#include <kotaka/paths/account.h>
#include <kotaka/paths/system.h>

static int query_user_class(string username)
{
	if (!username) {
		return 0;
	}

	if (username == "admin") {
		return 4;
	}

	if (!ACCOUNTD->query_is_registered(username)) {
		return 0;
	}

	if (KERNELD->access(username, "/", FULL_ACCESS)) {
		return 3;
	}

	if (sizeof( KERNELD->query_users() & ({ username }) )) {
		return 2;
	}

	return 1;
}
