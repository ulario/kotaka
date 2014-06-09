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
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

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

void bomb(int quota)
{
	int limit;

	limit = (int)sqrt((float)quota);

	CHANNELD->post_message("debug", "bomb", "Bombs this round: " + limit + "\nBombs left to go: " + quota);

	while (limit) {
		if (quota) {
			quota--;
			clone_object("obj/bomb");
		}

		limit--;
	}

	if (quota) {
		call_out("bomb", 0, quota);
	}
}

void upgrade_subsystem()
{
	ACCESS_CHECK(previous_program() == INITD);

	load();

	purge_orphans("Test");
}
