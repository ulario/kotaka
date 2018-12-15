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
#include <kotaka/log.h>
#include <kotaka/privilege.h>

inherit "/lib/string/sprint";

mapping tree;

static void create()
{
	tree = ([ ]);
}

void add_object_name(string name, object obj)
{
	string *parts;
	mapping map;
	int sz, i;
	string part;

	ACCESS_CHECK(SYSTEM());

	parts = explode(":" + name + ":", ":");

	if (sizeof(parts & ({ "" }))) {
		error("Invalid object name");
	}

	map = tree;

	sz = sizeof(parts);

	for (i = 0; i < sz - 1; i++) {
		part = parts[i];

		switch(typeof(map[part])) {
		case T_NIL:
			map[part] = ([ ]);

		case T_MAPPING:
			map = map[part];
			break;

		case T_OBJECT:
			error("Object in the way at " + implode(parts[0 .. i], ":"));
		}
	}

	if (map[parts[sz - 1]]) {
		error("Duplicate name");
	}

	map[parts[sz - 1]] = obj;
}

void remove_object_name(string name)
{
	string *parts;
	mapping *map;
	int sz, i;

	ACCESS_CHECK(SYSTEM());

	parts = explode(":" + name + ":", ":");

	if (sizeof(parts & ({ "" }))) {
		error("Invalid object name");
	}

	sz = sizeof(parts);

	map = allocate(sz);
	map[0] = tree;

	for (i = 0; i < sz - 1; i++) {
		string part;

		part = parts[i];

		map[i + 1] = map[i][part];

		switch(typeof(map[i][part])) {
		case T_NIL:
			return;

		case T_MAPPING:
			break;

		case T_STRING:
			error("Object in the way at " + implode(parts[0 .. i], ":"));
		}
	}

	if (map[sz - 1][parts[sz - 1]] == nil) {
		return;
	}

	map[sz - 1][parts[sz - 1]] = nil;

	while (i > 0) {
		if (map_sizeof(map[i]) == 0) {
			i--;
			map[i][parts[i]] = nil;
		} else {
			break;
		}
	}
}

string find_object_name(string name)
{
	string *parts;
	mapping map;
	int sz, i;

	parts = explode(":" + name + ":", ":");

	if (sizeof(parts & ({ "" }))) {
		error("Invalid object name");
	}

	sz = sizeof(parts);

	map = tree;

	for (i = 0; i < sz - 1; i++) {
		string part;

		part = parts[i];

		switch(typeof(map[part])) {
		case T_NIL:
			return nil;

		case T_MAPPING:
			map = map[part];
			break;

		case T_STRING:
			error("Object in the way at " + implode(parts[0 .. i], ":"));
		}
	}

	if (map) {
		return map[parts[sz - 1]];
	}
}

string *query_names(string name)
{
	mapping map;
	int sz, i;
	string *parts;
	string *keys;

	map = tree;

	if (name) {
		parts = explode(":" + name + ":", ":");

		if (sizeof(parts & ({ "" }))) {
			error("Invalid object name");
		}

		sz = sizeof(parts);

		for (i = 0; i < sz; i++) {
			string part;

			part = parts[i];

			switch(typeof(map[part])) {
			case T_NIL:
				return nil;

			case T_MAPPING:
				map = map[part];
				break;

			case T_STRING:
				error("Object in the way at " + implode(parts[0 .. i], ":"));
			}
		}
	}

	keys = map_indices(map);

	for (sz = sizeof(keys); --sz >= 0; ) {
		string key;

		key = keys[sz];

		if (typeof(map[key]) != T_OBJECT) {
			keys[sz] = nil;
		}
	}

	return keys - ({ nil });
}

string *query_directories(string name)
{
	mapping map;

	int sz, i;
	string *parts;
	string *keys;

	map = tree;

	if (name) {
		parts = explode(":" + name + ":", ":");

		if (sizeof(parts & ({ "" }))) {
			error("Invalid object name");
		}

		sz = sizeof(parts);

		for (i = 0; i < sz; i++) {
			string part;

			part = parts[i];

			switch(typeof(map[part])) {
			case T_NIL:
				return nil;

			case T_MAPPING:
				map = map[part];
				break;

			case T_STRING:
				error("Object in the way at " + implode(parts[0 .. i], ":"));
			}
		}
	}

	keys = map_indices(map);

	for (sz = sizeof(keys); --sz >= 0; ) {
		string key;

		key = keys[sz];

		if (typeof(map[key]) != T_MAPPING) {
			keys[sz] = nil;
		}
	}

	return keys - ({ nil });
}

void dump()
{
	LOGD->post_message("debug", LOG_DEBUG, "IDD Dump:\n" + hybrid_sprint(tree));
}
