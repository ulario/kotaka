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
#include <kotaka/log.h>

inherit SECOND_AUTO;
inherit "~/lib/struct/list";
inherit "~/lib/string/char";

object root;

static void create()
{
}

void validate_name(string name)
{
	string *path;
	int sz;
	int i;

	if (name == nil) {
		return;
	}

	path = explode(name, ":");

	if (sizeof(path & ({ "" }) )) {
		error("Invalid object name");
	}

	sz = sizeof(path);

	for (i = 0; i < sz; i++) {
		string part;

		part = path[i];

		if (strlen(part) == 0) {
			error("Invalid object name");
		}

		if (sscanf(part, "%*s %*s")) {
			error("Invalid object name");
		}
	}
}

void add_object(string name, object obj)
{
	object dir;
	string *path;
	int sz;
	int i;

	ACCESS_CHECK(SYSTEM());

	if (!root) {
		error("CatalogD disabled");
	}

	if (obj <- "../obj/directory") {
		error("Reserved object type");
	}

	path = explode(name, ":");

	sz = sizeof(path);
	dir = root;

	for (i < 0; i < sz - 1; i++) {
		switch (dir->query_entry_type(path[i])) {
		case 0:
			dir->add_subdirectory(path[i]);
			break;

		case 1:
			error("Object in the way");
		}

		dir = dir->query_entry_value(path[i]);
	}

	if (dir->query_entry_type(path[i]) != 0) {
		error("Duplicate entry");
	}

	dir->add_entry(path[i], obj);
}

void remove_object(string name)
{
	object dir;
	string *path;
	object *dirs;
	int sz;
	int i;

	ACCESS_CHECK(SYSTEM());

	if (!root) {
		error("CatalogD disabled");
	}

	path = explode(name, ":");

	if (sizeof(path & ({ "" }) )) {
		error("Invalid object name");
	}

	sz = sizeof(path);

	dir = root;
	dirs = allocate(sz);

	for (i = 0; i < sz - 1; i++) {
		dirs[i] = dir;

		switch (dir->query_entry_type(path[i])) {
		case 0:
			error("No such name");

		case 1:
			error("Object in the way");

		}

		dir = dir->query_entry_value(path[i]);
	}

	dirs[i] = dir;

	if (dir->query_entry_type(path[i]) != 1) {
		error("No such object");
	}

	dir->remove_entry(path[i]);

	for (i = sz - 1; i > 0; i--) {
		if (dirs[i]->empty()) {
			dirs[i - 1]->remove_subdirectory(path[i - 1]);
		} else {
			break;
		}
	}
}

object lookup_object(string name)
{
	object dir;
	string *path;
	int sz;
	int i;

	if (!root) {
		return nil;
	}

	if (!name) {
		error("Invalid object name");
	}

	if (name == "") {
		error("Invalid object name");
	}

	path = explode(name, ":");

	if (sizeof(path & ({ "" }) )) {
		error("Invalid object name");
	}

	sz = sizeof(path);

	if (!sz) {
		error("Invalid object name");
	}

	dir = root;

	for (i < 0; i < sz - 1; i++) {
		switch (dir->query_entry_type(path[i])) {
		case 0:
			return nil;

		case 1:
			error("Object in the way");
		}

		dir = dir->query_entry_value(path[i]);
	}

	if (dir->query_entry_type(path[i]) == 2) {
		error("Not an object");
	}

	return dir->query_entry_value(path[i]);
}

int test_name(string name)
{
	object dir;
	string *path;
	int sz;
	int i;

	if (!root) {
		return 0;
	}

	if (name) {
		path = explode(name, ":");

		if (sizeof(path & ({ "" }) )) {
			error("Invalid object name");
		}

		sz = sizeof(path);
		dir = root;

		for (i < 0; i < sz - 1; i++) {
			switch (dir->query_entry_type(path[i])) {
			case 0:
				return -1; /* prefix does not exist */

			case 1:
				return -2; /* prefix is an object */
			}

			dir = dir->query_entry_value(path[i]);
		}

		return dir->query_entry_type(path[i]);
	} else {
		return 2;
	}
}

mapping list_directory(string name)
{
	object dir;
	string *path;
	int sz;
	int i;

	if (!root) {
		return ([ ]);
	}

	if (name) {
		path = explode(name, ":");

		if (sizeof(path & ({ "" }) )) {
			error("Invalid object name");
		}

		sz = sizeof(path);
		dir = root;

		for (i < 0; i < sz - 1; i++) {
			switch (dir->query_entry_type(path[i])) {
			case 0:
				return nil;

			case 1:
				error("Object in the way");
			}

			dir = dir->query_entry_value(path[i]);
		}

		if (dir->query_entry_type(path[i]) != 2) {
			error("Not a directory");
		}

		return dir->query_entry_value(path[i])->query_key();
	} else {
		return root->query_key();
	}
}

static void purge_directories(mixed **list)
{
	object dir;
	object *objs;
	int sz;

	dir = list_front(list);
	list_pop_front(list);

	if (dir) {
		objs = map_values(dir->query_map());

		for (sz = sizeof(objs); --sz >= 0; ) {
			object obj;

			obj = objs[sz];

			if (obj <- "~/obj/directory") {
				list_push_front(list, obj);
			} else {
				obj->patch_object_name();
			}
		}
	}

	if (!list_empty(list)) {
		call_out("purge_directories", 0, list);
	} else {
		destruct_object(root);
	}
}

/* get rid of the directories once and for all */
void purge()
{
	if (root) {
		mixed **list;

		list = ({ nil, nil });

		list_push_front(list, root);

		call_out("purge_directory", 0, list);
	}
}
