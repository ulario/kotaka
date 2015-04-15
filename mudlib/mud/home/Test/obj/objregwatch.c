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
#include <config.h>
#include <kernel/rsrc.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/system.h>

string owner;
object cursor;
object master;

static void create(int clone)
{
	if (clone) {
		call_out("check", 0);
	}
}

void set_owner(string new_owner)
{
	ASSERT(!owner);

	owner = new_owner;
}

static void check()
{
	int count;
	object prev, next;

	if (!find_object("../sys/objregwatchd")) {
		destruct_object(this_object());

		return;
	}

	count = KERNELD->rsrc_get(owner, "objects")[RSRC_USAGE];

	if (!cursor) {
		cursor = KERNELD->first_link(owner);

		if (!cursor) {
			destruct_object(this_object());

			return;
		}
	}

	if (count > 120) {
		count = 120;
	}

	call_out("check", 60.0 / (float)count);

	prev = KERNELD->prev_link(cursor);
	next = KERNELD->next_link(cursor);

	if (KERNELD->next_link(prev) != cursor) {
		LOGD->post_message("test", LOG_EMERG, "Fatal error: ObjRegD corruption for " + owner);
		KERNELD->shutdown();
		error("internal error");
	}

	if (KERNELD->prev_link(next) != cursor) {
		LOGD->post_message("test", LOG_EMERG, "Fatal error: ObjRegD corruption for " + owner);
		KERNELD->shutdown();
		error("internal error");
	}

	cursor = next;

	if (!cursor) {
		LOGD->post_message("test", LOG_EMERG, "Fatal error: ObjRegD corruption for " + owner);
		KERNELD->shutdown();
		error("internal error");
	}
}
