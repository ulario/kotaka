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
#include <status.h>
#include <kotaka/paths/string.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;
inherit "~/lib/struct/list";

static void set_multimap(mapping multimap, int index, mixed value)
{
	int shift;
	int level;

	mapping submap;

	shift = index;

	while (shift & ~255) {
		shift >>= 8;
		level++;
	}

	submap = multimap[level];

	if (!submap) {
		multimap[level] = submap = ([ ]);
	}

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

static mixed query_multimap(mapping multimap, int index)
{
	int shift;
	int level;

	mapping submap;

	shift = index;

	while (shift & ~255) {
		shift >>= 8;
		level++;
	}

	submap = multimap[level];

	if (!submap) {
		return nil;
	}

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

private int compact_submap(mapping map, int level)
{
	if (level) {
		int *indices;
		mapping *maps;
		int sz;

		indices = map_indices(map);
		maps = map_values(map);

		for (sz = sizeof(indices); --sz >= 0; ) {
			if (compact_submap(maps[sz], level - 1) == 0) {
				map[indices[sz]] = nil;
			}
		}
	}

	return map_sizeof(map);
}

static void compact_multimap(mapping multimap)
{
	int *levels;
	mapping *maps;
	int sz;

	levels = map_indices(multimap);
	maps = map_values(multimap);

	for (sz = sizeof(levels); --sz >= 0; ) {
		if (compact_submap(maps[sz], levels[sz]) == 0) {
			multimap[levels[sz]] = nil;
		}
	}
}

private void build_list(mixed **list, int level, mapping map, int values)
{
	if (level) {
		int *levels;
		mapping *maps;

		maps = map_values(map);

		while (sizeof(maps)) {
			build_list(list, level - 1, maps[0], values);

			maps = maps[1 ..];
		}
	} else {
		mixed *data;

		if (values) {
			data = map_values(map);
		} else {
			data = map_indices(map);
		}

		while (sizeof(data)) {
			if (list_empty(list)) {
				object *left;
				int max;

				max = status(ST_ARRAYSIZE);

				if (sizeof(data) > max) {
					LOGD->post_message("system", LOG_NOTICE, "Empty list, overflow, tacking on " + max + " elements");

					list_push_back(list, data[0 .. max - 1]);
					data = data[max ..];
				} else {
					LOGD->post_message("system", LOG_NOTICE, "Empty list, tacking on " + sizeof(data) + " elements");

					list_push_back(list, data);
					data = ({ });
				}
			} else {
				mixed *last;
				int max;

				last = list_back_node(list);

				max = status(ST_ARRAYSIZE) - sizeof(last[1]);

				if (sizeof(data) > max) {
					LOGD->post_message("system", LOG_NOTICE, "Building list, last element has " + sizeof(last[1]) + ", overflow, tacking on " + max + " of " + sizeof(data));
					last[1] = last[1] + data[0 .. max - 1];
					data = data[max ..];

					/* obviously we have more to do */
					list_push_back(list, ({ }) );
				} else {
					LOGD->post_message("system", LOG_NOTICE, "Building list, last element has " + sizeof(last[1]) + ", tacking on " + sizeof(data));
					last[1] = last[1] + data;
					data = ({ });
				}
			}
		}
	}
}

private mixed **create_list(mapping map, int values)
{
	int sz;
	mixed **list;

	int *levels;
	mapping *maps;
	/* return a linked list with all the clones */

	if (!map) {
		return nil;
	}

	list = ({ nil, nil });

	levels = map_indices(map);
	maps = map_values(map);

	while (sizeof(levels)) {
		build_list(list, levels[0], maps[0], values);

		levels = levels[1 ..];
		maps = maps[1 ..];
	}

	return list;
}

static mixed **query_multimap_indices(mapping multimap)
{
	return create_list(multimap, 0);
}

static mixed **query_multimap_values(mapping multimap)
{
	return create_list(multimap, 1);
}
