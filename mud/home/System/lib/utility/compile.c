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
	mixed **files;
	string *names;
	mixed *objs;
	int *sizes;
	int index;

	if (dir == "/") {
		dir = "";
	}

	names = get_dir(dir + "/*")[0];

	for (index = 0; index < sizeof(names); index++) {
		mixed *info;
		string name;
		string path;

		name = names[index];

		info = file_info(dir + "/" + name);

		/* directory */
		if (info[0] == -2) {
			process_dir(dir + "/" + name, func);
			continue;
		}

		/* not a .c file */
		if (strlen(name) <= 2 || name[strlen(name) - 2 ..] != ".c") {
			continue;
		}

		name = name[.. strlen(name) - 3];
		path = dir + "/" + name;

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
				if (info[2]) {
					/* already loaded */
					continue;
				}
				break;

			case "recompile":
				if (!info[2]) {
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
			if (info[2]) {
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
