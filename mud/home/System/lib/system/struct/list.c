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

static mixed list_front(mixed **list)
{
	if (list[0]) {
		return list[0][1];
	} else {
		error("List empty");
	}
}

static mixed list_back(mixed **list)
{
	if (list[1]) {
		return list[1][1];
	} else {
		error("List empty");
	}
}

static mixed *list_front_node(mixed **list)
{
	if (list[0]) {
		return list[0];
	} else {
		error("List empty");
	}
}

static mixed *list_back_node(mixed **list)
{
	if (list[1]) {
		return list[1];
	} else {
		error("List empty");
	}
}

static mixed *list_prev_node(mixed *node)
{
	return node[0];
}

static mixed *list_next_node(mixed *node)
{
	return node[1];
}

static void list_push_front(mixed **list, mixed data)
{
	mixed *node;

	node = ({ nil, data, nil });

	if (list[0]) {
		node[2] = list[0];	/* new node points forward */
		list[0][0] = node;	/* old node points backward */
		list[0] = node;
	} else {
		list[0] = list[1] = node;
	}
}

static void list_push_back(mixed **list, mixed data)
{
	mixed *node;

	node = ({ nil, data, nil });

	if (list[1]) {
		node[0] = list[1];	/* new node points backward */
		list[1][2] = node;	/* old node points forward */
		list[1] = node;
	} else {
		list[0] = list[1] = node;
	}
}

static void list_pop_front(mixed **list)
{
	if (!list[0]) {
		error("List empty");
	}

	if (list[0] == list[1]) {
		list[0] = list[1] = nil;
	} else {
		list[0] = list[0][2];	/* advance */
		list[0][0] = nil;	/* snip */
	}
}

static void list_pop_back(mixed **list)
{
	if (!list[0]) {
		error("List empty");
	}

	if (list[0] == list[1]) {
		list[0] = list[1] = nil;
	} else {
		list[1] = list[1][0];	/* advance */
		list[1][2] = nil;	/* snip */
	}
}

static int list_empty(mixed **list)
{
	return !list[0];
}

static void list_append_string(mixed **list, string str)
{
	mixed *node;
	int max;
	int len;

	max = status(ST_STRSIZE);

	if (max > 4096) {
		max = 4096;
	}

	if (list_empty(list)) {
		list_push_front(list, "");
	}

	while (len = strlen(str)) {
		int spare;

		node = list_back_node(list);
		spare = max - strlen(node[1]);

		if (spare >= len) {
			node[1] += str;

			return;
		} else {
			if (spare > 0) {
				node[1] += str[0 .. spare - 1];
				str = str[spare ..];
			}

			list_push_back(list, "");
		}
	}
}
