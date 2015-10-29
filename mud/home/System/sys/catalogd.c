/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013, 2015  Raymond Jennings
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
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

object root;

static void create()
{
	root = clone_object("../obj/directory");
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

void dump()
{
	root->dump(0);
}

object lookup_object(string name)
{
	object dir;
	string *path;
	int sz;
	int i;

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

private void gather(string path, object dir, object deque)
{
	string *names;
	mapping key;
	int *vals;
	int sz;

	key = dir->query_key();
	names = map_indices(key);
	vals = map_values(key);

	for (sz = sizeof(names) - 1; sz >= 0; --sz) {
		switch(vals[sz]) {
		case 1:
			if (!dir->query_entry_value(names[sz])) {
				deque->push_back(
					path ?
					(path + ":" + names[sz]) :
					names[sz]);
			}
			break;

		case 2:
			gather(
				path ?
				path + ":" + names[sz] :
				names[sz], dir->query_entry_value(names[sz]), deque);
		}
	}
}

void fix()
{
	ACCESS_CHECK(SYSTEM());

	rlimits(0; -1) {
		object deque;

		deque = new_object(BIGSTRUCT_DEQUE_LWO);
		deque->claim();

		gather(nil, root, deque);

		while (!deque->empty()) {
			remove_object(deque->query_front());
			deque->pop_front();
		}
	}
}
