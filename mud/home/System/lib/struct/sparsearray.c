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

inherit SECOND_AUTO;
inherit "list";

static void sparsearray_set_element(mapping map, int index, mixed value)
{
	int shift;
	int level;

	mapping submap;

	shift = index;

	while (shift & ~255) {
		shift >>= 8;
		level++;
	}

	submap = map;

	while (level) {
		shift = index & (-1 << (level << 3));

		if (!submap[shift]) {
			submap[shift] = ([ ]);
		}

		submap = submap[shift];
		level--;
	}

	submap[index] = value;
}

static mixed sparsearray_query_element(mapping map, int index)
{
	int shift;
	int level;

	mapping submap;

	shift = index;

	while (shift & ~255) {
		shift >>= 8;
		level++;
	}

	submap = map;

	while (level) {
		shift = index & (-1 << (level << 3));

		if (!submap[shift]) {
			return nil;
		}

		submap = submap[shift];
		level--;
	}

	return submap[index];
}

private void build_list(mapping submap, int level, mixed **list, int values)
{
	int *indices;
	int i, sz;

	indices = map_indices(submap);
	sz = sizeof(indices);

	if (level) {
		for (i = 0; i < sz; i++) {
			build_list(submap[indices[i]], level - 1, list, values);
		}
	} else {
		for (i = 0; i < sz; i++) {
			list_push_back(list, values ? submap[indices[i]] : indices[i]);
		}
	}
}

private mixed **fetch_list(mapping map, int values)
{
	mixed **list;
	int level;
	int bit;
	mapping submap;

	list = ({ nil, nil });

	build_list(map[0 .. 0xff], 0, list, values);
	build_list(map[0x100 .. 0xffff], 1, list, values);
	build_list(map[0x10000 .. 0xffffff], 2, list, values);
	build_list(map[0x1000000 .. 0xffffffff], 3, list, values);

	return list;
}

static mixed **sparsearray_query_indices(mapping map)
{
	return fetch_list(map, 0);
}

static mixed **sparsearray_query_values(mapping map)
{
	return fetch_list(map, 1);
}
