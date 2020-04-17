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
#include <config.h>
#include <kotaka/paths/system.h>

inherit SECOND_AUTO;

/* load */
static void load_dir(string dir)
{
	string *names;
	int *sizes;
	int *times;
	mixed *objs;
	int sz;

	if (dir == "/") {
		dir = "";
	}

	({ names, sizes, times, objs }) = get_dir(dir + "/*");

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;
		string path;

		name = names[sz];

		if (sizes[sz] == -2) {
			/* directory */
			load_dir(dir + "/" + name);
			continue;
		}

		if (strlen(name) <= 2 || name[strlen(name) - 2 ..] != ".c") {
			/* not a .c file */
			continue;
		}

		path = dir + "/" + name[.. strlen(name) - 3];

		if (sscanf(path, "%*s" + INHERITABLE_SUBDIR + "%*s")) {
			/* inheritable */
			continue;
		}

		if (objs[sz]) {
			/* already loaded */
			continue;
		}

		compile_object(path);
	}
}

/* destruct */
static void destruct_dir(string dir)
{
	/* destruct if exists */
	string *names;
	int *sizes;
	int *times;
	mixed *objs;
	int sz;

	if (dir == "/") {
		dir = "";
	}

	({ names, sizes, times, objs }) = get_dir(dir + "/*");

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;
		string path;

		name = names[sz];

		if (sizes[sz] == -2) {
			/* directory */
			destruct_dir(dir + "/" + name);
			continue;
		}

		if (strlen(name) <= 2 || name[strlen(name) - 2 ..] != ".c") {
			/* not a .c file */
			continue;
		}

		path = dir + "/" + name[.. strlen(name) - 3];

		if (objs[sz]) {
			destruct_object(path);
		}
	}
}

/* compile, even if already loaded */
static void compile_dir(string dir)
{
	string *names;
	int *sizes;
	int *times;
	mixed *objs;
	int sz;

	if (dir == "/") {
		dir = "";
	}

	({ names, sizes, times, objs }) = get_dir(dir + "/*");

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;
		string path;

		name = names[sz];

		if (sizes[sz] == -2) {
			/* directory */
			compile_dir(dir + "/" + name);
			continue;
		}

		if (strlen(name) <= 2 || name[strlen(name) - 2 ..] != ".c") {
			/* not a .c file */
			continue;
		}

		path = dir + "/" + name[.. strlen(name) - 3];

		if (sscanf(path, "%*s" + INHERITABLE_SUBDIR + "%*s")) {
			/* inheritable */
			continue;
		}

		compile_object(path);
	}
}

/* recompile only if loaded */
static void recompile_dir(string dir)
{
	string *names;
	int *sizes;
	int *times;
	mixed *objs;
	int sz;

	if (dir == "/") {
		dir = "";
	}

	({ names, sizes, times, objs }) = get_dir(dir + "/*");

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;
		string path;

		name = names[sz];

		if (sizes[sz] == -2) {
			/* directory */
			recompile_dir(dir + "/" + name);
			continue;
		}

		if (strlen(name) <= 2 || name[strlen(name) - 2 ..] != ".c") {
			/* not a .c file */
			continue;
		}

		path = dir + "/" + name[.. strlen(name) - 3];

		if (sscanf(path, "%*s" + INHERITABLE_SUBDIR + "%*s")) {
			/* inheritable */
			continue;
		}

		if (!objs[sz]) {
			/* not yet loaded */
			continue;
		}

		compile_object(path);
	}
}

/* destruct libraries */
static void purge_dir(string dir)
{
	string *names;
	int *sizes;
	int *times;
	mixed *objs;
	int sz;

	if (dir == "/") {
		dir = "";
	}

	({ names, sizes, times, objs }) = get_dir(dir + "/*");

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;
		string path;

		name = names[sz];

		if (sizes[sz] == -2) {
			/* directory */
			purge_dir(dir + "/" + name);
			continue;
		}

		if (strlen(name) <= 2 || name[strlen(name) - 2 ..] != ".c") {
			/* not a .c file */
			continue;
		}

		path = dir + "/" + name[.. strlen(name) - 3];

		if (!sscanf(path, "%*s" + INHERITABLE_SUBDIR)) {
			continue;
		}

		if (objs[sz]) {
			destruct_object(path);
		}
	}
}
