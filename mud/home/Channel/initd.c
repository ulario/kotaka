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
#include <kernel/kernel.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	load_dir("sys");
}

private void set_limits()
{
	KERNELD->rsrc_set_limit("Channel", "callouts", 10);
}

static void create()
{
	KERNELD->set_global_access("Channel", 1);

	MODULED->boot_module("String");

	set_limits();

	load();

	CHANNELD->restore();
}

void prepare_reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	CHANNELD->save();
}

void reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	CHANNELD->restore();
}

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);

	set_limits();
}
