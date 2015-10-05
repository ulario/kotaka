/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2015  Raymond Jennings
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
#include <kotaka/log.h>
#include <status.h>

inherit SECOND_AUTO;
inherit UTILITY_COMPILE;

static void create()
{
	call_out("pass", 0);
}

static void pass()
{
	destruct_dir("~Bigstruct/lib", 1);
	compile_dir("~Bigstruct/lwo", 1);
	compile_dir("~Bigstruct/obj", 1);
	compile_dir("~Bigstruct/sys", 1);

	compile_object(PROGRAMD);
	compile_object(CALLOUTD);
	compile_object(SUSPENDD);

	while (CALLOUTD->query_suspend_count()) {
		CALLOUTD->release_callouts();
	}

	while (SYSTEM_USERD->query_blocked()) {
		SYSTEM_USERD->unblock_connections();
	}

	call_out("pass_2", 0);
}

static void pass_2()
{
	mixed *callouts;

	callouts = status(OBJECTD, O_CALLOUTS);

	if (sizeof(callouts)) {
		LOGD->post_message("debug", LOG_DEBUG, "ObjectD is busy, waiting to rebuild it.");

		call_out("pass_2", 0.1);

		return;
	}

	OBJECTD->disable();
	compile_object(OBJECTD);
	OBJECTD->enable();

	call_out("pass_3", 0);
}

static void pass_3()
{
	compile_object(OBJECTD);

	INITD->upgrade_system_3();

	destruct_object(this_object());
}
