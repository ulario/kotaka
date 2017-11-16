/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/system.h>

inherit SECOND_AUTO;

void wipe(string path)
{
	mixed *info;

	info = file_info(path);
	ASSERT(info);

	if (info[0] == -2) {
		for (;;) {
			mixed **dir;
			string *names;
			int sz;

			dir = get_dir(path + "/*");
			names = dir[0];
			sz = sizeof(names);

			if (sz == 0) {
				remove_dir(path);
				return;
			}

			for (; --sz >= 0; ) {
				wipe(path + "/" + names[sz]);
			}
		}
	} else {
		remove_file(path);
	}
}
