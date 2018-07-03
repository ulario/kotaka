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

string *rows;
int width, height;

static void create(int clone)
{
	if (clone) {
		rows = ({ });
	}
}

void set_size(int new_width, int new_height)
{
	int top;
	int bytes, new_bytes;
	int i;

	bytes = (width + 7) >> 3;
	new_bytes = (new_width + 7) >> 3;

	if (new_height > height) {
		int i;
		string nulls;

		rows += allocate(new_height - height);
		nulls = STRINGD->nulls(new_bytes);

		for (i = height; i < new_height; i++) {
			rows[i] = nulls;
		}
	}

	if (new_height < height) {
		rows = rows[0 .. new_height - 1];
	}

	top = height;

	if (new_bytes > bytes) {
		int i;
		string suffix;

		/* extend each row */
		suffix = STRINGD->nulls(new_bytes - bytes);

		for (i = 0; i < top; i++) {
			rows[i] += suffix;
		}
	}

	if (new_bytes < bytes) {
		int i;

		for (i = 0; i < top; i++) {
			rows[i] = rows[i][0 .. new_bytes - 1];
		}
	}

	if (new_width < width) {
		int i;

		for (i = 0; i < top; i++) {
			rows[i][new_bytes - 1] &= (255 >> (new_width & 7));
		}
	}

	width = new_width;
	height = new_height;
}

int query_width()
{
	return width;
}

int query_height()
{
	return height;
}

void set_cell(int x, int y, int b)
{
	if (x < 0 || x >= width || y < 0 || y >= height) {
		error("Coordinates out of bounds");
	}

	if (b) {
		rows[y][x >> 3] |= 1 << (x & 7);
	} else {
		rows[y][x >> 3] &= ~(1 << (x & 7));
	}
}

int query_cell(int x, int y)
{
	int s;

	if (x < 0 || x >= width || y < 0 || y >= height) {
		error("Coordinates out of bounds");
	}

	s = x & 7;

	return (rows[y][x >> 3] & (1 << s)) >> s;
}
