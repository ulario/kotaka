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
	load_dir("lwo");
	load_dir("obj");
	load_dir("sys");
}

private void set_limits()
{
	KERNELD->rsrc_set_limit("Test", "ticks", 500000000);
}

static void create()
{
	MODULED->boot_module("Bigstruct");

	set_limits();

	load();
}

static void destruct()
{
	destruct_dir("lwo");
	destruct_dir("obj");
	destruct_dir("sys");
}

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);

	set_limits();

	load();

	purge_orphans("Test");
}

string query_patcher(string path)
{
	LOGD->post_message("debug", LOG_DEBUG, "Asked for patcher for " + path);
}
