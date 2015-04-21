/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
	KERNELD->set_global_access("Account", 1);

	load();
}

void prepare_reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	ACCOUNTD->force_save();
	BAND->force_save();
}

void reboot()
{
	ACCESS_CHECK(previous_program() == MODULED);

	ACCOUNTD->force_restore();
	BAND->force_restore();
}

static void upgrade_module_2()
{
	"sys/accountd"->upgrade();
}

void upgrade_module()
{
	mixed *info;

	ACCESS_CHECK(previous_program() == MODULED);

	load();
	compile_object("sys/accountd");
	purge_orphans("Account");

	info = SECRETD->file_info("accounts");

	if (info && info[0] != -2) {
		SECRETD->remove_file("account");
		SECRETD->rename_file("accounts", "account");
	}

	call_out("upgrade_module_2", 0);
}
