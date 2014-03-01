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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit SECOND_AUTO;

object first;
mapping clones;
int count;

static void create(int clone)
{
	if (clone) {
		clones = ([ ]);
	}
}

void add_clone(object obj)
{
	ACCESS_CHECK(previous_program() == CLONED);

	if (clones) {
		ASSERT(!clones[obj]);

		if (count == status(ST_ARRAYSIZE)) {
			clones = nil;
		} else {
			clones[obj] = 1;
		}
	}

	count++;
}

void remove_clone(object obj)
{
	ACCESS_CHECK(previous_program() == CLONED);

	if (clones) {
		ASSERT(clones[obj]);
		clones[obj] = nil;
	}

	count--;
}

void set_first_clone(object obj)
{
	ACCESS_CHECK(previous_program() == CLONED);

	first = obj;
}

object query_first_clone()
{
	return first;
}

int query_clone_count()
{
	return count;
}

object *query_clones()
{
	return map_indices(clones);
}
