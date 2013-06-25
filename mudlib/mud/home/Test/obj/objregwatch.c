/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kernel/kernel.h>
#include <kotaka/paths.h>
#include <kernel/rsrc.h>

string owner;
object cursor;
object master;

static void create(int clone)
{
	if (clone) {
		call_out("check", 0);
	}
}

void activate(object first)
{
	master = find_object(USR_DIR + "/Test/sys/objregwatchd");

	if (previous_object() != master) {
		error("Access denied");
	}

	cursor = first;

	owner = first->query_owner();
}

static void check()
{
	int count;

	if (!master || !owner) {
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

	call_out("check", SUBD->rnd() * 0.5 + (10.0 / (float)count));

	cursor = KERNELD->next_link(cursor);

	if (!cursor) {
		DRIVER->message("Fatal error: ObjRegD corruption for " + owner);
		shutdown();
	}
}
