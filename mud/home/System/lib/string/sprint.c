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
#include <type.h>

inherit SECOND_AUTO;
inherit "escape";

string simple_sprint(mixed data, varargs mapping seen, int nodup)
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
		if (!nodup) {
			if (seen[data] != nil) {
				return "@" + seen[data];
			}

			seen[data] = map_sizeof(seen);
		}

		if (sizeof(data) == 0)
			return "({ })";

		tmp = "({ ";
		for (iter = 0; iter < sizeof(data); iter++) {
			tmp += simple_sprint(data[iter], seen);
			if (iter < sizeof(data) - 1) {
				tmp += ", ";
			}
		}
		return tmp + " })";

	case T_MAPPING:
		if (!nodup) {
			if (seen[data] != nil) {
				return "@" + seen[data];
			}

			seen[data] = map_sizeof(seen);
		}

		if (map_sizeof(data) == 0)
			return "([ ])";

		arr = map_indices(data);
		tmp = "([ ";
		for (iter = 0; iter < sizeof(arr); iter++) {
			tmp += simple_sprint(arr[iter], seen) + " : " +
				simple_sprint(data[arr[iter]], seen);
			if (iter != sizeof(arr) - 1)
				tmp += ", ";
		}
		return tmp + " ])";

	case T_OBJECT:
		return "<" + object_name(data) + ">";
	}
}
