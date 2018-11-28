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
#include <type.h>
#include <kotaka/paths/system.h>

inherit SECOND_AUTO;
inherit "~/lib/struct/sparsearray";

mapping data;

static void create(int clone)
{
	if (clone) {
		data = ([ ]);
	}
}

void set_element(int index, mixed value)
{
	sparsearray_set_element(data, index, value);
}

mixed query_element(int index)
{
	return sparsearray_query_element(data, index);
}

mixed **query_indices()
{
	return sparsearray_query_indices(data);
}

mixed **query_values()
{
	return sparsearray_query_values(data);
}
