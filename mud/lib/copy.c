/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2022  Raymond Jennings
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

mixed deep_copy(mixed in, varargs mapping dupes)
{
	switch (typeof(in)) {
	case T_NIL:
		return nil;

	case T_INT:
	case T_FLOAT:
	case T_STRING:
		return in;

	default:
		if (typeof(in) == T_OBJECT
			&& sscanf(object_name(in), "%*s#-1") == 0) {
			return in;
		}
	}

	if (!dupes) {
		dupes = ([ ]);
	}

	if (!dupes[in]) {
		int index, sz;
		mixed *ind;
		mixed *val;

		switch (typeof(in)) {
		case T_OBJECT:
			{
				mixed data;
				object new_obj;

				new_obj = new_object(in);
				dupes[in] = new_obj;
				new_obj->finish_deep_copy(dupes);
			}
			break;

		case T_ARRAY:
			{
				int sz;

				sz = sizeof(in);
				dupes[in] = allocate(sz);

				for (index = 0; index < sz; index++) {
					dupes[in][index] =
						deep_copy(in[index], dupes);
				}
			}
			break;

		case T_MAPPING:
			{
				int sz;

				dupes[in] = ([ ]);

				ind = map_indices(in);
				val = map_values(in);
				sz = sizeof(ind);

				for (index = 0; index < sz; index++) {
					ind[index] = deep_copy(ind[index], dupes);
					val[index] = deep_copy(val[index], dupes);

					dupes[in][ind[index]] = val[index];
				}
			}
		}
	}

	return dupes[in];
}
