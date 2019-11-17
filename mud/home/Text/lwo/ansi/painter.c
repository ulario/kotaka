/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019  Raymond Jennings
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
#include <kotaka/log.h>
#include <kotaka/privilege.h>

inherit "/lib/string/char";

string *stack;
mapping layers;

int size_x, size_y;

static void create(int clone)
{
	if (clone) {
		stack = ({ });
		layers = ([ ]);
	}
}

void set_size(int dx, int dy)
{
	size_x = dx;
	size_y = dy;
}

void add_layer(string name)
{
	stack += ({ name });

	layers[name] = new_object("layer");
}

void set_layer_size(string name, int sx, int sy)
{
	layers[name]->set_size(sx, sy);
}

void set_layer_position(string name, int px, int py)
{
	layers[name]->set_position(px, py);
}

object query_layer(string name)
{
	ACCESS_CHECK(TEXT());

	return layers[name];
}

object create_gc()
{
	object gc;

	gc = new_object("gc");
	gc->set_painter(this_object());

	return gc;
}

string *render()
{
	int i, sz;
	int x, y;

	string *chars;

	sz = sizeof(stack);

	for (i = 0; i < sz; i++) {
		int sx, sy;
		int px, py;

		string *lmask;
		string *lchars;

		object layer;

		layer = layers[stack[i]];

		({ sx, sy }) = layer->query_size();
		({ px, py }) = layer->query_position();

		lmask = layer->query_mask();
		lchars = layer->query_chars();

		for (y = 0; y < sy; y++) {
			for (x = 0; x < sx; x++) {
				if (lmask[y][x >> 3] & (1 << (x & 7))) {
					chars[y + py][x + px] = lchars[y][x];
				}
			}
		}
	}

	return chars;
}

string *render_color()
{
	int i, sz;
	int x, y;

	string *chars;
	string *colors;
	string *buffers;

	chars = allocate(size_y);
	colors = allocate(size_y);

	for (y = 0; y < size_y; y++) {
		chars[y] = spaces(size_x);
		colors[y] = chars(0x07, size_x);
	}

	sz = sizeof(stack);

	for (i = 0; i < sz; i++) {
		int sx, sy;
		int px, py;
		int mx, my;

		string *lmask;
		string *lchars;
		string *lcolors;

		object layer;

		layer = layers[stack[i]];

		({ sx, sy }) = layer->query_size();
		({ px, py }) = layer->query_position();

		lmask = layer->query_mask();
		lchars = layer->query_chars();
		lcolors = layer->query_colors();

		if (px + sx > size_x) {
			sx = size_x - px;
		}

		if (py + sy > size_y) {
			sy = size_y - py;
		}

		if (px < 0) {
			mx = -px;
		}

		if (py < 0) {
			my = -py;
		}

		for (y = my; y < sy; y++) {
			for (x = mx; x < sx; x++) {
				if (lmask[y][x >> 3] & (1 << (x & 7))) {
					if (lcolors[y][x] & 0x80) {
						colors[y + py][x + px] =
							(0x0f & lcolors[y][x])
							| (0xf0 & colors[y + py][x + px]);
					} else {
						colors[y + py][x + px] = lcolors[y][x];
					}

					chars[y + py][x + px] = lchars[y][x];
				}
			}
		}
	}

	buffers = allocate(size_y);

	for (y = 0; y < size_y; y++) {
		int delta;
		int color;
		string buffer;
		buffer = "";

		color = 0x7;

		for (x = 0; x < size_x; x++) {
			int new_color;
			int dirty;

			new_color = colors[y][x];
			delta = x == 0 ? 0xf : color ^ new_color;
			color = new_color;

			if (delta) {
				buffer += "\033[";

				if (delta & 0x8) {
					dirty = 1;

					if (new_color & 0x8) {
						buffer += "1";
					} else {
						buffer += "22";
					}
				}

				if (delta & 0x7) {
					int fcolor;

					if (dirty) {
						buffer += ";";
					}

					dirty = 1;

					fcolor = new_color & 0x7;

					buffer += "3" + (fcolor == 0 ? 9 : fcolor);
				}

				if (delta & 0x70) {
					int bcolor;

					if (dirty) {
						buffer += ";";
					}

					dirty = 1;

					bcolor = (new_color >> 4) & 0x7;

					buffer += "4" + (bcolor == 0 ? 9 : bcolor);
				}

				buffer += "m";
			}

			buffer += chars[y][x .. x];
		}

		buffer += "\033[0m";
		color = 0x07;

		buffers[y] = buffer;
	}

	return buffers;
}
