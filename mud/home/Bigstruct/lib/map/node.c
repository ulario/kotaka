/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kotaka/privilege.h>

inherit "../bintree/node";

mapping map;
mixed low_key;
int size;

static void create()
{
	::create();

	map = ([ ]);
}

int query_mass()
{
	ACCESS_CHECK(BIGSTRUCT());

	return map_sizeof(map);
}

mixed query_low_key()
{
	ACCESS_CHECK(BIGSTRUCT());

	return low_key;
}

void set_low_key(mixed key)
{
	ACCESS_CHECK(BIGSTRUCT());

	low_key = key;
}

void reset_low_key()
{
	mixed *keys;

	ACCESS_CHECK(BIGSTRUCT());

	keys = map_indices(map);

	if (sizeof(keys)) {
		low_key = keys[0];
	} else {
		low_key = nil;
	}
}

mapping query_map()
{
	ACCESS_CHECK(BIGSTRUCT());

	return map;
}

void set_map(mapping new_map)
{
	ACCESS_CHECK(BIGSTRUCT());

	map = new_map;
}

void insert_data(mapping data)
{
	ACCESS_CHECK(BIGSTRUCT());

	map += data;
}

void delete_data(mixed *keys)
{
	ACCESS_CHECK(BIGSTRUCT());

	map -= keys;
}

int query_size()
{
	ACCESS_CHECK(BIGSTRUCT());

	return size;
}

void set_size(int new_size)
{
	ACCESS_CHECK(BIGSTRUCT());

	size = new_size;
}
