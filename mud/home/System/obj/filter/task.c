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
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;
inherit LIB_FILTER;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	INITD->begin_task();

	return ::login(str);
}

void logout(int quit)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	INITD->begin_task();

	::logout(quit);
}

int receive_message(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	INITD->begin_task();

	return ::receive_message(str);
}

int message_done()
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	INITD->begin_task();

	return ::message_done();
}

void receive_datagram(string packet)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	INITD->begin_task();

	::receive_datagram(packet);
}
