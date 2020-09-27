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
#include <kotaka/log.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	MODULED->boot_module("Kotaka");

	load_dir("sys");

	"sys/verbd"->sync_help();
}

private void set_limits()
{
	reset_limits();

	KERNELD->rsrc_set_limit("Verb", "ticks", 2000000);
}

static void reload_verb_help()
{
	LOGD->post_message("system", LOG_NOTICE, "Reloading verb help");

	"sys/verbd"->sync_help();
}

static void create()
{
	KERNELD->set_global_access("Verb", 1);

	set_limits();

	load();

	reload_verb_help();
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	set_limits();
}

void upgrade_build()
{
	ACCESS_CHECK(previous_program() == MODULED);

	compile_dir("sys");

	reload_verb_help();
}

void booted_module(string module)
{
	ACCESS_CHECK(previous_program() == MODULED);

	if (module == "Kotaka") {
		reload_verb_help();
	}
}
