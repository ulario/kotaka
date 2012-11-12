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
#include <kotaka/paths.h>
#include <kotaka/log.h>

string *chars;
string *colors;

int pen_x, pen_y;
int size_x, size_y;
int color;

static void create(int clone)
{
}

void start(int dx, int dy)
{
	int i;

	size_x = dx;
	size_y = dy;

	chars = allocate(size_y);
	colors = allocate(size_y);

	for (i = 0; i < size_y; i++) {
		chars[i] = STRINGD->spaces(size_x);
		colors[i] = STRINGD->chars('\007', size_x);
	}
}

/* FR FG FB FI BR BG BB BT */
void set_color(int new_color)
{
	color = new_color & 0xFF;
}

void move_pen(int new_x, int new_y)
{
	pen_x = new_x;
	pen_y = new_y;
}

void draw(string brush)
{
	int i;
	int sz;

	sz = strlen(brush);

	if (pen_y < 0 || pen_y >= size_y || pen_x >= size_x || pen_x + sz < 0) {
		/* completely out of bounds */
		pen_x += sz;
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

		pen_x++;
	}
}

string *render()
{
	return chars[..];
}

string *render_color()
{
	int i, j;
	int color, delta;
	string *buffers;

	buffers = allocate(size_y);
	color = 0x7;

	for (i = 0; i < size_y; i++) {
		string buffer;
		buffer = "";

		for (j = 0; j < size_x; j++) {
			int new_color;
			int dirty;

			new_color = colors[i][j];
			delta = color ^ new_color;
			color = new_color;

			if (delta) {
				buffer += "\033[";
				if (delta & 0x8) {
					dirty = 1;
					if (new_color & 0x8) {
						buffer += "1";
					} else {
						buffer += "21";
					}
				}
				if (delta & 0x7) {
					if (dirty) {
						buffer += ";";
					}
					dirty = 1;
					buffer += "3" + (new_color & 0x7);
				}
				if (delta & 0x70) {
					if (dirty) {
						buffer += ";";
					}
					dirty = 1;
					buffer += "4" + ((new_color >> 4) & 0x7);
				}
				buffer += "m";
			}

			buffer += chars[i][j .. j];
		}

		if (color != 0x07) {
			buffer += "\033[0m";
			color = 0x07;
		}

		buffers[i] = buffer;
	}

	return buffers;
}
