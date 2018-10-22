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
#include <kotaka/paths/string.h>
#include <kotaka/privilege.h>

string *chars;
string *colors;
string *mask;

int pos_x, pos_y;
int size_x, size_y;

static void create(int clone)
{
	if (!chars) {
		chars = ({ });
		colors = ({ });
		mask = ({ });
	}
}

void set_position(int x, int y)
{
	ACCESS_CHECK(ANSI());

	pos_x = x;
	pos_y = y;
}

int *query_position()
{
	ACCESS_CHECK(ANSI());

	return ({ pos_x, pos_y });
}

private void change_length(int h, int o, int n)
{
	int i;
	int ob, nb;

	ob = (o + 7) >> 3;
	nb = (n + 7) >> 3;

	if (n > o) {
		/* lengthen */
		for (i = 0; i < h; i++) {
			chars[i] += STRINGD->chars('?', n - o);
			colors[i] += STRINGD->chars(0x87, n - o);
			mask[i] += STRINGD->nulls(nb - ob);
			/* new bits are supposed to be null anyway */
		}
	} else {
		/* shorten */
		for (i = 0; i < h; i++) {
			chars[i] = chars[i][0 .. n - 1];
			colors[i] = colors[i][0 .. n - 1];
			mask[i] = mask[i][0 .. nb - 1];
		}

		if (n & 7 != 0) {
			for (i = 0; i < h; i++) {
				mask[i][nb - 1] &= 255 >> (8 - (n & 7));
			}
		}
	}
}

void set_size(int x, int y)
{
	int i;

	ACCESS_CHECK(ANSI());

	if (!chars) {
		chars = ({ });
		colors = ({ });
		mask = ({ });
	}

	if (y > size_y) {
		change_length(size_y, size_x, x);

		chars += allocate(y - size_y);
		colors += allocate(y - size_y);
		mask += allocate(y - size_y);

		for (i = size_y; i < y; i++) {
			chars[i] = STRINGD->spaces(x);
			colors[i] = STRINGD->chars(0x87, x);
			mask[i] = STRINGD->nulls((x + 7) >> 3);
		}
	} else if (y < size_y) {
		chars = chars[0 .. y - 1];
		colors = colors[0 .. y - 1];
		mask = mask[0 .. y - 1];
		change_length(y, size_x, x);
	} else {
		change_length(y, size_x, x);
	}

	size_x = x;
	size_y = y;
}

int *query_size()
{
	ACCESS_CHECK(ANSI());

	return ({ size_x, size_y });
}

void draw(string brush, int pen_x, int pen_y, int color)
{
	int i;
	int sz;

	ACCESS_CHECK(ANSI());

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
			new_color = (colors[pen_y][pen_x] & 0xf0) | (color & 0xf);
		} else {
			new_color = color;
		}

		colors[pen_y][pen_x] = new_color;
		mask[pen_y][pen_x >> 3] |= 1 << (pen_x & 7);

		pen_x++;
	}
}

void erase(int length, int pen_x, int pen_y)
{
	int i;
	int sz;

	ACCESS_CHECK(ANSI());

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
	ACCESS_CHECK(ANSI());

	return colors;
}

string *query_chars()
{
	ACCESS_CHECK(ANSI());

	return chars;
}

string *query_mask()
{
	ACCESS_CHECK(ANSI());

	return mask;
}
