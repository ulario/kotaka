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
	load_dir("sys");
}

private void set_limits()
{
	reset_limits();

	KERNELD->rsrc_set_limit("Verb", "ticks", 2000000);
}

static void create()
{
	set_limits();

	load();
}

static void reload_verb_help()
{
	LOGD->post_message("system", LOG_NOTICE, "Reloading verb help");

	"sys/verbd"->sync_help();
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

	call_out("reload_verb_help", 0);
}
