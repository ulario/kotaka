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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

mapping key;
mapping map;

static void create(int clone)
{
	if (clone) {
		key = ([ ]);
		map = ([ ]);
	}
}

void add_subdirectory(string name)
{
	ACCESS_CHECK(previous_program() == CATALOGD);

	if (key[name]) {
		error("Duplicate entry");
	}

	key[name] = 2;
	map[name] = clone_object("directory");
}

void remove_subdirectory(string name)
{
	object subdir;

	ACCESS_CHECK(previous_program() == CATALOGD);

	if (key[name] != 2) {
		error("Not a directory");
	}

	if (!map[name]->empty()) {
		error("Directory not empty");
	}

	key[name] = nil;
	destruct_object(map[name]);
}

void add_entry(string name, object obj)
{
	ACCESS_CHECK(previous_program() == CATALOGD);

	if (key[name]) {
		error("Duplicate entry");
	}

	key[name] = 1;
	map[name] = obj;
}

void remove_entry(string name)
{
	ACCESS_CHECK(previous_program() == CATALOGD);

	if (key[name] != 1) {
		error("Not an object");
	}

	key[name] = nil;
	map[name] = nil;
}

int query_entry_type(string name)
{
	ACCESS_CHECK(previous_program() == CATALOGD);

	if (!key[name]) {
		return 0;
	}

	return key[name];
}

object query_entry_value(string name)
{
	ACCESS_CHECK(previous_program() == CATALOGD);

	return map[name];
}

int empty()
{
	ACCESS_CHECK(SYSTEM());

	return !map_sizeof(key);
}

int mass()
{
	ACCESS_CHECK(previous_program() == CATALOGD);

	return map_sizeof(key);
}

mapping query_key()
{
	ACCESS_CHECK(previous_program() == CATALOGD);

	return key[..];
}

mapping query_map()
{
	ACCESS_CHECK(previous_program() == CATALOGD);

	return map;
}
