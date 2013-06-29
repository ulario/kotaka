/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>

string *chars;
string *colors;
string *mask;

int pos_x, pos_y;
int size_x, size_y;

void set_pos(int x, int y)
{
	ACCESS_CHECK(TEXT());

	pos_x = x;
	pos_y = y;
}

int *query_pos()
{
	ACCESS_CHECK(TEXT());

	return ({ pos_x, pos_y });
}

void set_size(int dx, int dy)
{
	int i;

	ACCESS_CHECK(TEXT());

	size_x = dx;
	size_y = dy;

	chars = allocate(size_y);
	colors = allocate(size_y);
	mask = allocate(size_y);

	for (i = 0; i < size_y; i++) {
		chars[i] = STRINGD->spaces(size_x);
		colors[i] = STRINGD->chars(0x87, size_x);
		mask[i] = STRINGD->nulls((size_x + 7) >> 3);
	}
}

void draw(string brush, int pen_x, int pen_y, int color)
{
	int i;
	int sz;

	ACCESS_CHECK(TEXT());

	sz = strlen(brush);

	if (pen_y < 0 || pen_y >= size_y || pen_x >= size_x || pen_x + sz < 0) {
		/* completely out of bounds */
		return;
	}

	if (pen_x < 0) {
		/* head truncated */
		brush = brush[-pen_x ..];
		sz += pen_x;
		pen_x = 0;
	} else if (pen_x + sz > size_x) {
		/* tail truncated */
		sz = size_x - pen_x;
		brush = brush[0 .. sz - 1];
	}

	while (i < sz) {
		int new_color;
		chars[pen_y][pen_x] = brush[i++];

		if (color & 0x80) {
			/* transparent background */
			new_color = (colors[pen_y][pen_x] & 0x70) | (color & 0xF);
		} else {
			new_color = color;
		}

		colors[pen_y][pen_x] = new_color;
		mask[pen_y][pen_x >> 3] |= 1 << (pen_x & 7);

		pen_x++;
	}
}

void erase(int length, int pen_x, int pen_y, int color)
{
	int i;
	int sz;

	ACCESS_CHECK(TEXT());

	sz = length;

	if (pen_y < 0 || pen_y >= size_y || pen_x >= size_x || pen_x + sz < 0) {
		/* completely out of bounds */
		return;
	}

	if (pen_x < 0) {
		/* head truncated */
		sz += pen_x;
		pen_x = 0;
	} else if (pen_x + sz > size_x) {
		/* tail truncated */
		sz = size_x - pen_x;
	}

	while (i < sz) {
		mask[pen_y][pen_x >> 3] &= ~(1 << (pen_x & 7));

		pen_x++;
		i++;
	}
}

string *query_colors()
{
	ACCESS_CHECK(TEXT());

	return colors;
}

string *query_chars()
{
	ACCESS_CHECK(TEXT());

	return chars;
}

string *query_mask()
{
	ACCESS_CHECK(TEXT());

	return mask;
}
