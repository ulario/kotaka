/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kotaka/paths/account.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	load_dir("obj");
	load_dir("sys");
}

static void create()
{
	MODULED->boot_module("String");

	KERNELD->set_global_access("Account", 1);

	load();

	ACCOUNTD->restore();
	BAND->restore();
}

void prepare_reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	ACCOUNTD->save();
	BAND->save();
}

void reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	ACCOUNTD->restore();
	BAND->restore();
}

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);

	load();

	purge_orphans("Account");
}
