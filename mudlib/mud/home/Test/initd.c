/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014  Raymond Jennings
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

inherit LIB_INITD;
inherit UTILITY_COMPILE;

private void load()
{
	load_dir("obj", 1);
	load_dir("sys", 1);
}

static void create()
{
	KERNELD->set_global_access("Test", 1);

	load();
}

void bomb()
{
	int done;
	int ticks;

	ticks = status(ST_TICKS);

	ASSERT(ticks > 0);

	while (ticks - status(ST_TICKS) < 20000000) {
		if (status(ST_OTABSIZE) - status(ST_NOBJECTS) <= 1024) {
			return;
		}
		clone_object("obj/bomb");
	}

	if (!done) {
		call_out("bomb", 0);
	}
}

void upgrade_module()
{
	ACCESS_CHECK(previous_program() == MODULED);

	load();

	purge_orphans("Test");
}
