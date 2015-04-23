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
#include <kotaka/assert.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>

#define TICKS 200000

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	load_dir("obj", 1);
}

static void create()
{
	KERNELD->set_global_access("Test", 1);

	load();
}

void bomb(int quota)
{
	rlimits (0; TICKS + 20000) {
		int ticks;

		ticks = status(ST_TICKS);

		while (ticks - status(ST_TICKS) < TICKS && quota > 0) {
			clone_object("obj/bomb");
			quota--;
		}
	}

	if (quota > 0) {
		call_out("bomb", 0, quota);
	}
}

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);

	load();

	purge_orphans("Test");
}
