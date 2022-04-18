/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020  Raymond Jennings
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

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	load_dir("misc");
	load_dir("lwo");
	load_dir("sys");
}

private void set_limits()
{
	reset_limits();
}

static void create()
{
	set_limits();

	load();
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	set_limits();
}

void upgrade_purge()
{
	ACCESS_CHECK(previous_program() == MODULED);

	purge_orphans(nil);

	purge_dir("lib");
}

void upgrade_build()
{
	ACCESS_CHECK(previous_program() == MODULED);

	compile_dir("misc");
	compile_dir("lwo");
	compile_dir("sys");
}

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);

	load();
}
