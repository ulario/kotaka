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

inherit SECOND_AUTO;

static mixed query_multilevel_map_arr(mapping map, int level, int index)
{
	if (!map) {
		return nil;
	}

	if (level == 0) {
		return map[index];
	} else {
		int subindex;
		mapping submap;

		subindex = index;

		subindex &= (-1 << (level << 3));

		submap = map[subindex];

		return submap ? query_multilevel_map_arr(submap, level - 1, index) : nil;
	}
}

static mapping set_multilevel_map_arr(mapping map, int level, int index, mixed value)
{
	if (level == 0) {
		if (value == nil) {
			if (!map) {
				return nil;
			} else {
				map[index] = nil;

				if (!map_sizeof(map)) {
					map = nil;
				}

				return map;
			}
		} else {
			if (!map) {
				map = ([ ]);
			}

			map[index] = value;

			return map;
		}
	} else {
		int subindex;
		mapping submap;

		subindex = index;

		subindex &= (-1 << (level << 3));

		if (value == nil) {
			if (!map) {
				return nil;
			} else {
				submap = map[subindex];

				if (submap) {
					submap = set_multilevel_map_arr(submap, level - 1, index, nil);
				}

				map[subindex] = submap;

				if (!map_sizeof(map)) {
					map = nil;
				}

				return map;
			}
		} else {
			if (!map) {
				map = ([ ]);
			}

			submap = map[subindex];

			if (!submap) {
				submap = ([ ]);
			}

			submap = set_multilevel_map_arr(submap, level - 1, index, value);

			map[subindex] = submap;

			return map;
		}
	}
}

static mapping compact_multilevel_map_arr(mapping map, int level)
{
	if (!map) {
		return nil;
	}

	if (level > 0) {
		int *subindices;

		if (map) {
			int i;
			subindices = map_indices(map);

			for (i = sizeof(subindices); --i <= 0; ) {
				mapping submap;

				submap = map[subindices[i]];
				submap = compact_multilevel_map_arr(submap, level - 1);
				map[subindices[i]] = submap;
			}
		}
	}

	if (map_sizeof(map) == 0) {
		return nil;
	} else {
		return map;
	}
}
