/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/assert.h>

object painter;
object layer;

int color;
int pen_x, pen_y;	/* relative to origin */
int off_x, off_y;	/* location of origin on layer */
int clip_lx, clip_ly, clip_hx, clip_hy;	/* relative to origin */

void set_painter(object new_painter)
{
	ASSERT(!painter);

	painter = new_painter;
}

void set_layer(string name)
{
	layer = painter->query_layer(name);
}

void set_offset(int x, int y)
{
	off_x = x;
	off_y = y;
}

void set_clip(int lx, int ly, int hx, int hy)
{
	clip_lx = lx;
	clip_ly = ly;
	clip_hx = hx;
	clip_hy = hy;
}

/*
0x01 = foreground red
0x02 = foreground green
0x04 = foreground blue
0x08 = foreground intense
0x10 = background red
0x20 = background green
0x40 = background blue
0x80 = background transparent
*/
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
	int sz;

	sz = strlen(brush);

	if (pen_y < clip_ly || pen_y > clip_hy || pen_x > clip_hx || pen_x + sz < clip_lx) {
		/* completely out of bounds */
		pen_x += sz;
		return;
	}

	if (pen_x < clip_lx) {
		/* head truncated */
		brush = brush[clip_lx - pen_x ..];
		sz -= clip_lx - pen_x;
		pen_x = clip_lx;
	} else if (pen_x + sz > clip_hx) {
		/* tail truncated */
		sz = clip_hx - pen_x + 1;
		brush = brush[0 .. sz - 1];
	}

	layer->draw(brush, pen_x + off_x, pen_y + off_y, color);

	pen_x += sz;
}

void erase(int length)
{
	if (pen_y < clip_ly || pen_y > clip_hy || pen_x > clip_hx || pen_x + length < clip_lx) {
		/* completely out of bounds */
		pen_x += length;
		return;
	}

	if (pen_x < clip_lx) {
		/* head truncated */
		length -= clip_lx - pen_x;
		pen_x = clip_lx;
	} else if (pen_x + length > clip_hx) {
		/* tail truncated */
		length = clip_hx - pen_x + 1;
	}

	layer->erase(length, pen_x + off_x, pen_y + off_y);

	pen_x += length;
}
