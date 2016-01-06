/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2015  Raymond Jennings
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
#include <type.h>

inherit SECOND_AUTO;

string replace(string input, string from, string to)
{
	string *arr;

	arr = explode(from + input + from, from);

	return implode(arr, to);
}

string quote_unescape(string input)
{
	input = replace(input, "\\t", "\t");
	input = replace(input, "\\r", "\r");
	input = replace(input, "\\n", "\n");
	input = replace(input, "\\\"", "\"");
	input = replace(input, "\\0", "\000");
	input = replace(input, "\\\\", "\\");

	return input;
}

string quote_escape(string input)
{
	input = replace(input, "\\", "\\\\");
	input = replace(input, "\000", "\\0");
	input = replace(input, "\"", "\\\"");
	input = replace(input, "\n", "\\n");
	input = replace(input, "\r", "\\r");
	input = replace(input, "\t", "\\t");

	return input;
}

string mixed_sprint(mixed data, varargs mapping seen)
{
	int iter;
	string tmp;
	mixed *arr;

	if (!seen) {
		seen = ([ ]);
	}

	switch (typeof(data)) {
	case T_NIL:
		return "nil";

	case T_STRING:
		return "\"" + quote_escape(data) + "\"";

	case T_INT:
		return (string)data;

	case T_FLOAT:
		/* decimal point is required */
		{
			string mantissa;
			string exponent;
			string str;

			str = (string)data;

			if (!sscanf(str, "%se%s", mantissa, exponent)) {
				mantissa = str;
				exponent = "";
			} else {
				exponent = "e" + exponent;
			}

			if (!sscanf(mantissa, "%*s.")) {
				mantissa += ".0";
			}

			return mantissa + exponent;
		}

	case T_ARRAY:
		if (seen[data] != nil) {
			return "@" + seen[data];
		}

		seen[data] = map_sizeof(seen);

		if (sizeof(data) == 0) {
			return "({ })";
		}

		tmp = "({ ";

		for (iter = 0; iter < sizeof(data); iter++) {
			tmp += mixed_sprint(data[iter], seen);

			if (iter < sizeof(data) - 1) {
				tmp += ", ";
			}
		}

		return tmp + " })";

	case T_MAPPING:
		if (seen[data] != nil) {
			return "@" + seen[data];
		}

		seen[data] = map_sizeof(seen);

		if (map_sizeof(data) == 0) {
			return "([ ])";
		}

		arr = map_indices(data);
		tmp = "([ ";
		for (iter = 0; iter < sizeof(arr); iter++) {
			tmp += mixed_sprint(arr[iter], seen) + " : " +
				mixed_sprint(data[arr[iter]], seen);
			if (iter != sizeof(arr) - 1)
				tmp += ", ";
		}
		return tmp + " ])";

	case T_OBJECT:
		return "<" + object_name(data) + ">";
	}
}

string initd_of(string module)
{
	if (module) {
		return USR_DIR + "/" + module + "/initd";
	} else {
		return "/initd";
	}
}

void validate_key(mixed key)
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
