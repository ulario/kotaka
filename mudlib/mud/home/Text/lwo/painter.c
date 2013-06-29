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
#include <kotaka/privilege.h>

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
	object *obj;
	int i, sz;

	size_x = dx;
	size_y = dy;

	obj = map_values(layers);

	sz = sizeof(obj);

	for (i = 0; i < sz; i++) {
		obj[i]->set_size(dx, dy);
	}
}

void add_layer(string name)
{
	stack += ({ name });
	layers[name] = new_object("layer");
	layers[name]->set_size(size_x, size_y);
}

object get_layer(string name)
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

	string **lchars;
	string **lmask;

	sz = sizeof(stack);

	chars = allocate(size_y);
	lchars = allocate(sz);
	lmask = allocate(sz);

	for (i = sz - 1; i >= 0; i--) {
		lchars[i] = layers[stack[i]]->query_chars();
		lmask[i] = layers[stack[i]]->query_mask();
	}

	for (y = 0; y < size_y; y++) {
		chars[y] = STRINGD->spaces(size_x);

		for (x = 0; i < size_x; x++) {
			for (i = sz - 1; i >= 0; i--) {
				if (lmask[i][y][x >> 3] & (1 << (x & 7))) {
					/* hit */
					chars[y][x] = lchars[i][y][x];

					break;
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
	int delta;
	int color;

	string *chars;
	string *colors;
	string *buffers;

	string **lchars;
	string **lcolors;
	string **lmask;

	sz = sizeof(stack);

	chars = allocate(size_y);
	colors = allocate(size_y);
	buffers = allocate(size_y);
	lchars = allocate(sz);
	lcolors = allocate(sz);
	lmask = allocate(sz);

	for (i = 0; i < sz; i++) {
		lchars[i] = layers[stack[i]]->query_chars();
		lcolors[i] = layers[stack[i]]->query_colors();
		lmask[i] = layers[stack[i]]->query_mask();
	}

	for (y = 0; y < size_y; y++) {
		chars[y] = STRINGD->spaces(size_x);
		colors[y] = STRINGD->chars(0x80, size_x);

		for (x = 0; x < size_x; x++) {
			for (i = 0; i < sz; i++) {

				if (lmask[i][y][x >> 3] & (1 << (x & 7))) {
					/* hit */
					chars[y][x] = lchars[i][y][x];
					colors[y][x] = lcolors[i][y][x];
				}
			}
		}
	}

	for (y = 0; y < size_y; y++) {
		string buffer;
		buffer = "";

		for (x = 0; x < size_x; x++) {
			int new_color;
			int dirty;

			new_color = colors[y][x];
			delta = color ^ new_color;
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

			buffer += chars[y][x .. x];
		}

		if (color != 0x07) {
			buffer += "\033[0m";
			color = 0x07;
		}

		buffers[y] = buffer;
	}

	return buffers;
}
