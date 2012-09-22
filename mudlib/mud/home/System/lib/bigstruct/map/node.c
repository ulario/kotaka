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
#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

#include <status.h>

inherit SECOND_AUTO;
inherit "../bintree/node";

/* only root may manipulate nodes */

mapping map;
mixed low_key;
int size;

static void create()
{
	::create();

	map = ([ ]);
}

static void destruct()
{
	::destruct();
}

int get_mass()
{
	check_caller();

	return map_sizeof(map);
}

mixed get_low_key()
{
	check_caller();

	return low_key;
}

void set_low_key(mixed key)
{
	check_caller();

	low_key = key;
}

void reset_low_key()
{
	mixed *keys;

	check_caller();

	keys = map_indices(map);

	if (sizeof(keys)) {
		low_key = keys[0];
	} else {
		low_key = nil;
	}
}

mapping get_map()
{
	check_caller();

	return map;
}

void set_map(mapping new_map)
{
	check_caller();

	map = new_map;
}

void insert_data(mapping data)
{
	check_caller();

	map += data;
}

void delete_data(mixed *keys)
{
	check_caller();

	map -= keys;
}

int get_size()
{
	check_caller();

	return size;
}

void set_size(int new_size)
{
	check_caller();

	size = new_size;
}
