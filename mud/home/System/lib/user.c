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
#include <kernel/user.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;
inherit user LIB_USER;

static void create()
{
	user::create();
}

static void set_mode(int new_mode)
{
	if (this_object() && query_conn()) {
		query_conn()->set_mode(new_mode);
	}
}

static void redirect(object user, string str)
{
	SYSTEM_USERD->intercept_redirect(user, str);
}

void system_redirect(object user, string str)
{
	ACCESS_CHECK(SYSTEM());

	::redirect(user, str);
}
