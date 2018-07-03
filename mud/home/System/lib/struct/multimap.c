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
