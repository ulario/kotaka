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
#include <type.h>

inherit "~System/lib/string/sprint";
inherit "char";

string mixed_sprint(mixed data, varargs mapping seen, int nodup);

string sprint_object(object obj, varargs mapping seen, int nodup)
{
	string path;
	string oname;

	if (sscanf(object_name(obj), "%s#-1", path)) {
		if (seen[obj] != nil) {
			return "@" + seen[obj];
		}

		if (!nodup) {
			seen[obj] = map_sizeof(seen);
		}

		if (function_object("sprint_save", obj)) {
			return "(< <" + path + ">: " + mixed_sprint(obj->sprint_save(), seen, nodup) + ">)";
		} else {
			return "<" + object_name(obj) + ">";
		}
	} else if (function_object("query_object_name", obj) && (oname = obj->query_object_name())) {
		return "<" + oname + ">";
	} else {
		return "<" + object_name(obj) + ">";
	}
}

string mixed_sprint(mixed data, varargs mapping seen, int nodup)
{
	int iter;
	string tmp;
	mixed *arr;

	if (!seen) {
		seen = ([ ]);
	}

	switch (typeof(data)) {
	case T_NIL:
	case T_STRING:
	case T_INT:
	case T_FLOAT:
		return simple_sprint(data, seen, nodup);

	case T_ARRAY:
		if (seen[data] != nil) {
			return "@" + seen[data];
		}

		if (!nodup) {
			seen[data] = map_sizeof(seen);
		}

		if (sizeof(data) == 0)
			return "({ })";

		tmp = "({ ";
		for (iter = 0; iter < sizeof(data); iter++) {
			tmp += mixed_sprint(data[iter], seen, nodup);
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

		if (map_sizeof(data) == 0)
			return "([ ])";

		arr = map_indices(data);
		tmp = "([ ";
		for (iter = 0; iter < sizeof(arr); iter++) {
			tmp += mixed_sprint(arr[iter], seen, nodup) + " : " +
				mixed_sprint(data[arr[iter]], seen, nodup);
			if (iter != sizeof(arr) - 1)
				tmp += ", ";
		}
		return tmp + " ])";

	case T_OBJECT:
		return sprint_object(data, seen, nodup);
	}
}

string tree_sprint(mixed data, varargs int indent, mapping seen, int nodup)
{
	string ind;

	ind = "";

	if (!seen)
		seen = ([ ]);

	while (strlen(ind) < indent)
		ind += " ";

	switch (typeof(data)) {
	case T_INT:
	case T_NIL:
	case T_STRING:
	case T_FLOAT:
	case T_OBJECT:
		return mixed_sprint(data, seen, nodup);

	case T_ARRAY:
		if (seen[data] != nil) {
			return "@" + seen[data];
		}

		if (!nodup) {
			seen[data] = map_sizeof(seen);
		}

		if (sizeof(data) == 0)
			return "({ })";

		{
			string *parts;
			int index;

			parts = allocate(sizeof(data));

			for (index = 0; index < sizeof(data); index++) {
				parts[index] =
					"  " + tree_sprint(data[index],
					indent + 2, seen, nodup);
			}

			return "({\n" + ind + implode(parts,
				",\n" + ind) + "\n" + ind + "})";
		}

	case T_MAPPING:
		if (seen[data] != nil) {
			return "@" + seen[data];
		}

		if (!nodup) {
			seen[data] = map_sizeof(seen);
		}

		if (map_sizeof(data) == 0)
			return "([ ])";
		{
			mixed *indices;
			string *parts;
			int index;

			parts = allocate(map_sizeof(data));
			indices = map_indices(data);

			for (index = 0; index < map_sizeof(data); index++) {
				parts[index] =
					"  " + mixed_sprint(indices[index], seen, nodup) +
					" :\n" + ind + "    " +
					tree_sprint(data[indices[index]],
					indent + 4, seen, nodup);
			}

			return "([\n" + ind + implode(parts,
				",\n" + ind) + "\n" + ind + "])";
		}
	}
}

int is_simple(mixed data)
{
	int sz;

	switch (typeof(data)) {
	case T_INT:
	case T_NIL:
	case T_STRING:
	case T_FLOAT:
	case T_OBJECT:
		return 1;

	case T_ARRAY:
		sz = sizeof(data);
		return sz == 0;

	case T_MAPPING:
		sz = map_sizeof(data);
		return sz == 0;
	}
}

int is_flat(mixed data)
{
	int sz;
	int i;

	if (is_simple(data)) {
		return 1;
	}

	switch (typeof(data)) {
	case T_ARRAY:
		sz = sizeof(data);

		for (i = 0; i < sz; i++) {
			if (!is_simple(data[i])) {
				return 0;
			}
		}

		return 1;

	case T_MAPPING:
		sz = map_sizeof(data);

		{
			mixed *ind;
			mixed *val;


			ind = map_indices(data);
			val = map_values(data);

			for (i = 0; i < sz; i++) {
				if (!is_simple(ind[i])) {
					return 0;
				}
				if (!is_simple(val[i])) {
					return 0;
				}
			}
		}
		return 0;
	}
}

string hybrid_sprint(mixed data, varargs int indent, mapping seen, int nodup)
{
	string ind;

	ind = spaces(indent);

	if (!seen)
		seen = ([ ]);

	switch (typeof(data)) {
	case T_INT:
	case T_NIL:
	case T_STRING:
	case T_FLOAT:
	case T_OBJECT:
		return mixed_sprint(data, seen, nodup);

	case T_ARRAY:
		if (seen[data] != nil) {
			return "#" + seen[data];
		}

		if (sizeof(data) == 0)
			return "({ })";

		if (is_flat(data)) {
			return mixed_sprint(data, seen, nodup);
		}

		if (!nodup) {
			seen[data] = map_sizeof(seen);
		}

		{
			string *parts;
			int index;

			parts = allocate(sizeof(data));

			for (index = 0; index < sizeof(data); index++) {
				parts[index] =
					"  " + hybrid_sprint(data[index],
					indent + 2, seen, nodup);
			}

			return "({\n" + ind + implode(parts,
				",\n" + ind) + "\n" + ind + "})";
		}

	case T_MAPPING:
		if (seen[data] != nil) {
			return "@" + seen[data];
		}

		if (map_sizeof(data) == 0) {
			return "([ ])";
		}

		if (is_flat(data)) {
			return mixed_sprint(data, seen, nodup);
		}

		if (!nodup) {
			seen[data] = map_sizeof(seen);
		}

		{
			mixed *indices;
			string *parts;
			int index;

			parts = allocate(map_sizeof(data));
			indices = map_indices(data);

			for (index = 0; index < map_sizeof(data); index++) {
				parts[index] =
					"  " + mixed_sprint(indices[index], seen, nodup) +
					" :" + (is_simple(data[indices[index]]) ? " " : "\n" + ind + "    ") +
						hybrid_sprint(data[indices[index]],
					indent + 4, seen, nodup);
			}

			return "([\n" + ind + implode(parts,
				",\n" + ind) + "\n" + ind + "])";
		}
	}
}
