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

static void process_dir(string dir, string func)
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

		/* directory */
		if (sizes[sz] == -2) {
			process_dir(dir + "/" + name, func);
			continue;
		}

		/* not a .c file */
		if (strlen(name) <= 2 || name[strlen(name) - 2 ..] != ".c") {
			continue;
		}

		path = dir + "/" + name[.. strlen(name) - 3];

		switch(func) {
		case "load":
		case "compile":
		case "recompile":
			if (sscanf(path, "%*s" + INHERITABLE_SUBDIR + "%*s")) {
				/* inheritable */
				continue;
			}

			switch(func) {
			case "load":
				if (objs[sz]) {
					/* already loaded */
					continue;
				}
				break;

			case "recompile":
				if (!objs[sz]) {
					/* not loaded */
					continue;
				}
				break;
			}

			compile_object(path);
			break;

		case "purge":
			if (!sscanf(path, "%*s" + INHERITABLE_SUBDIR + "%*s")) {
				/* not inheritable */
				continue;
			}
			/* fall through */

		case "destruct":
			if (objs[sz]) {
				destruct_object(path);
			}
			break;
		}
	}
}

static void load_dir(string dir)
{
	process_dir(dir, "load");
}

static void destruct_dir(string dir)
{
	process_dir(dir, "destruct");
}

static void recompile_dir(string dir)
{
	process_dir(dir, "recompile");
}

static void compile_dir(string dir)
{
	process_dir(dir, "compile");
}

static void purge_dir(string dir)
{
	process_dir(dir, "purge");
}
