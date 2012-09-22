/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/bigstruct.h>
#include <kotaka/log.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

static void create()
{
	load_dir("obj", 1);
	load_dir("sys", 1);
}

void full_reset()
{
	object turkeylist;
	object cursor;
	object first;
	object this;

	ACCESS_CHECK(PRIVILEGED());

	turkeylist = new_object(BIGSTRUCT_DEQUE_LWO);

	this = this_object();
	cursor = KERNELD->first_link("Help");
	first = cursor;

	do {
		turkeylist->push_back(cursor);
		cursor = KERNELD->next_link(cursor);
	} while (cursor != first);

	while (!turkeylist->empty()) {
		object turkey;

		turkey = turkeylist->get_front();
		turkeylist->pop_front();

		if (!turkey || turkey == this) {
			/* don't self destruct or double destruct */
			continue;
		}

		destruct_object(turkey);
	}

	load_dir("obj", 1);
	load_dir("sys", 1);
}
