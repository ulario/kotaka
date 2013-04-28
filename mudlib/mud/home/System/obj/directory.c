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
#include <kotaka/paths.h>
#include <kotaka/log.h>
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

	destruct_object(map[name]);
	key[name] = nil;
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
	ACCESS_CHECK(previous_program() == CATALOGD);

	return !map_sizeof(key);
}

void dump(int level)
{
	string *names;
	int *types;
	object *values;
	int sz, i;

	ACCESS_CHECK(CATALOG());

	names = map_indices(key);
	types = map_values(key);
	values = map_values(map);

	sz = sizeof(names);

	if (!sz) {
		LOGD->post_message("catalog", LOG_DEBUG,
			STRINGD->spaces(level * 2) + "(empty)");
		return;
	}

	for (i = 0; i < sz; i++) {
		switch(types[i]) {
		case 1:
			LOGD->post_message("catalog", LOG_DEBUG,
				STRINGD->spaces(level * 4) + names[i] + ";");
			break;
		case 2:
			if (values[i]->mass()) {
				LOGD->post_message("catalog", LOG_DEBUG,
					STRINGD->spaces(level * 4) + names[i] + " {");
				values[i]->dump(level + 1);
				LOGD->post_message("catalog", LOG_DEBUG,
					STRINGD->spaces(level * 4) + "}");
			} else {
				LOGD->post_message("catalog", LOG_DEBUG,
					STRINGD->spaces(level * 4) + names[i] + " { }");
			}
			break;
		}
	}
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
