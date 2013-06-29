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

object layer;

int size_x, size_y;

static void create(int clone)
{
	if (clone) {
		layer = new_object("layer");
	}
}

void set_size(int dx, int dy)
{
	size_x = dx;
	size_y = dy;

	layer->set_size(dx, dy);
}

object create_gc()
{
	object gc;

	gc = new_object("gc");
	gc->set_layer(layer);

	return gc;
}

string *render()
{
	return layer->query_chars();
}

string *render_color()
{
	int i, j;
	int color, delta;

	string *colors;
	string *chars;
	string *buffers;

	colors = layer->query_colors();
	chars = layer->query_chars();

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
