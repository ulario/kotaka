/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2015  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>

inherit SECOND_AUTO;
inherit UTILITY_COMPILE;

static void create()
{
	SUSPENDD->queue_work("pass");
}

void pass()
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

	destruct_object("~/sys/swapd");
	destruct_object("~/sys/dumpd");

	compile_object("~/sys/swapd");
	compile_object("~/sys/dumpd");

	SUSPENDD->queue_work("pass_2");
}

void pass_2()
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

	INITD->upgrade_system_3();

	destruct_object(this_object());
}