/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019, 2020, 2022  Raymond Jennings
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
/**********/
/* NOTICE */
/**********/
/* The functionality of ChannelD is to be migrated to the Kotaka module */
#include <kernel/kernel.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	MODULED->boot_module("String");

	load_dir("sys");
}

private void set_limits()
{
	reset_limits();
}

static void create()
{
	KERNELD->set_global_access("Channel", 1);

	set_limits();

	load();

	CHANNELD->restore();
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	set_limits();
}

void upgrade_check()
{
	ACCESS_CHECK(previous_program() == MODULED);
}
