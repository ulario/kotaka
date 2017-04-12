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
#include <type.h>

private void validate_key(mixed key)
{
	switch(typeof(key)) {
	case T_INT:
	case T_FLOAT:
	case T_STRING:
		return;

	case T_NIL:
	case T_MAPPING:
	case T_ARRAY:
		error("Invalid key type");

	case T_OBJECT:
		if (sscanf(object_name(key), "%*s#-1")) {
			error("Invalid key type");
		}
	}
}

mapping set_tiered_map(mapping map, mixed args ...)
{
	mixed key;

	key = args[0];
	validate_key(key);

	if (sizeof(args) == 2) {
		mixed value;

		value = args[1];

		if (value == nil) {
			if (!map) {
				return nil;
			}

			map[key] = nil;

			if (map_sizeof(map) == 0) {
				map = nil;
			}
		} else {
			if (!map) {
				map = ([ ]);
			}

			map[key] = value;
		}

		return map;
	} else {
		mapping submap;
		mixed value;

		args = args[1 ..];
		value = args[sizeof(args) - 1];

		if (!map) {
			if (value == nil) {
				return nil;
			} else {
				map = ([ ]);
			}
		}

		submap = map[key];

		if (!submap) {
			submap = ([ ]);
		}

		submap = set_tiered_map(submap, args ...);

		map[key] = submap;

		if (!map_sizeof(map)) {
			map = nil;
		}

		return map;
	}
}

mixed query_tiered_map(mapping map, mixed args ...)
{
	mixed key;

	key = args[0];
	validate_key(key);

	if (sizeof(args) == 1) {
		return map ? map[key] : nil;
	} else {
		mapping submap;

		if (!map) {
			return nil;
		}

		submap = map[key];

		if (submap) {
			return query_tiered_map(submap, args[1 ..] ...);
		} else {
			return nil;
		}
	}
}
