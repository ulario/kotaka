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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	MODULED->boot_module("Bigstruct");

	load_dir("lwo");
	load_dir("obj");
	load_dir("sys");
}

private void set_limits()
{
	reset_limits();

	KERNELD->rsrc_set_limit("Test", "ticks", 2000000000);
}

static void test()
{
	"sys/testd"->schedule_tests();
}

static void create()
{
	set_limits();

	load();

	call_out("test", 0);
}

static void destruct()
{
	destruct_dir("lwo");
	destruct_dir("obj");
	destruct_dir("sys");
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	set_limits();
}
