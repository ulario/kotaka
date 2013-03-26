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
#include <kotaka/log.h>
#include <kotaka/paths.h>

object root;

static void create()
{
	root = clone_object("../obj/directory");
}

void add_object(string name, object obj)
{
	object dir;
	string *path;
	int sz;
	int i;

	path = explode(name, ":");

	if (sizeof(path & ({ "" }) )) {
		error("Invalid object name");
	}

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

	if (dir->query_entry_type(path[i]) != 1) {
		error("Not an object");
	}

	return dir->query_entry_value(path[i]);
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

		return dir->query_key();
	} else {
		return root->query_key();
	}
}
