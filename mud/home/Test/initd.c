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

void test()
{
	mixed diff;
	mixed *mtime1, *mtime2;

	mtime1 = millitime();
	"sys/bigstruct"->test();
	mtime2 = millitime();

	diff = mtime2[0] - mtime1[0];
	diff = (float)diff + mtime2[1] - mtime1[1];

	LOGD->post_message("system", LOG_NOTICE, "Bigstruct test completed in " + diff + " seconds");

	mtime1 = mtime2;
	"sys/struct"->test();
	mtime2 = millitime();

	diff = mtime2[0] - mtime1[0];
	diff = (float)diff + mtime2[1] - mtime1[1];

	LOGD->post_message("system", LOG_NOTICE, "Struct test completed in " + diff + " seconds, average time per element is " + diff / 1000000.0);

	mtime1 = mtime2;
	"sys/sysstruct"->test();

	mtime2 = millitime();

	diff = mtime2[0] - mtime1[0];
	diff = (float)diff + mtime2[1] - mtime1[1];

	LOGD->post_message("system", LOG_NOTICE, "Sysstruct test completed in " + diff + " seconds");
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
