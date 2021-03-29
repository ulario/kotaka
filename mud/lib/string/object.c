/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2021  Raymond Jennings
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

static object parse_object(string str)
{
	string suffix;
	string *ids;
	object obj;

	sscanf(str, "%s;%s", str, suffix);

	obj = find_object(str);

	if (!obj) {
		obj = IDD->find_object_by_name(str);
	}

	if (!obj) {
		error("No such object");
	}

	if (!suffix) {
		return obj;
	}

	ids = explode(suffix, ";");

	while (sizeof(ids)) {
		obj = obj->find_by_id(ids[0]);

		if (!obj) {
			error("No such object");
		}

		ids = ids[1 ..];
	}

	return obj;
}
