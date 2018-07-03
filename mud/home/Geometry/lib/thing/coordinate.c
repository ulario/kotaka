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
#include <kotaka/paths/geometry.h>
#include <kotaka/paths/thing.h>
#include <kotaka/paths/text.h>
#include <kotaka/log.h>

inherit LIB_THING;

string query_coordinate_system()
{
	return query_property("coordinate_system");
}

object query_outer_origin()
{
	object env;

	env = query_environment();

	while (env) {
		string s;

		s = env->query_property("coordinate_system");

		if (s) {
			return env;
		}

		env = env->query_environment();
	}
}

object query_inner_origin()
{
	if (query_coordinate_system()) {
		return this_object();
	} else {
		return query_outer_origin();
	}
}
