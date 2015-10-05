/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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

inherit SECOND_AUTO;

#define CMD_DESTRUCT	1
#define CMD_LOAD	2
#define CMD_RECOMPILE	3

#define FLAG_RECURSE	1
#define FLAG_CATCH	2

static void process_dir(string dir, int cmd, int flags)
{
	mixed **files;
	string *names;
	mixed *objs;
	int *sizes;
	int index;

	names = get_dir(dir + "/*")[0];

	for (index = 0; index < sizeof(names); index++) {
		mixed *info;
		string name;

		name = names[index];

		info = file_info(dir + "/" + name);

		if (info[0] == -2 && (flags & 1)) {
			process_dir(dir + "/" + name, cmd, flags);
			continue;
		}

		if (strlen(name) <= 2 || name[strlen(name) - 2 ..] != ".c") {
			continue;
		}

		name = name[ .. strlen(name) - 3];

		switch(cmd) {
		case 1:
			if (info[2]) {
				if (flags & 2) {
					catch {
						destruct_object(dir + "/" + name);
					}
				} else {
					destruct_object(dir + "/" + name);
				}
			}
			break;

		case 2:
			if (flags & 2) {
				catch {
					load_object(dir + "/" + name);
				}
			} else {
				load_object(dir + "/" + name);
			}
			break;

		case 3:
			if (info[2]) {
				compile_object(dir + "/" + name);
			}
		}
	}
}

static void load_dir(string dir, varargs int flags)
{
	process_dir(dir, CMD_LOAD, flags);
}

static void destruct_dir(string dir, varargs int flags)
{
	process_dir(dir, CMD_DESTRUCT, flags);
}

static void recompile_dir(string dir, varargs int flags)
{
	process_dir(dir, CMD_RECOMPILE, flags);
}
