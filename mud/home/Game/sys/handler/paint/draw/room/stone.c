/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/geometry.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/text.h>

#define RED	1
#define GREEN	2
#define BLUE	4
#define BOLD	8

void on_paint_text(object gc, object obj, object viewer)
{
	int dx, dy, dz;
	int lx, ly, hx, hy;
	mixed sx, sy;

	({ dx, dy, dz }) = GEOMETRY_SUBD->query_position_difference(viewer, obj);

	gc->set_layer("view");

	lx = dx;
	ly = dy;
	sx = obj->query_property("size_x");
	sy = obj->query_property("size_y");

	if (!sx) {
		sx = 0;
	}
	if (!sy) {
		sy = 0;
	}

	hx = dx + sx - 1;
	hy = dy + sy - 1;

	if (obj->is_container_of(viewer)) {
		int i;

		gc->set_color(0x08);
		for (i = -8; i <= 8; i++) {
			gc->move_pen(-8, i);
			gc->draw(STRINGD->chars('#', 17));
		}

		/* floor */
		gc->set_color(0x08);
		for (i = ly; i <= hy; i++) {
			gc->move_pen(lx, i);
			gc->draw(STRINGD->chars('.', sx));
		}

		/* corners */
		gc->set_color(0x07);
		gc->move_pen(lx - 1, ly - 1);
		gc->draw("+");
		gc->move_pen(hx + 1, ly - 1);
		gc->draw("+");
		gc->move_pen(lx - 1, hy + 1);
		gc->draw("+");
		gc->move_pen(hx + 1, hy + 1);
		gc->draw("+");

		/* walls */
		gc->move_pen(lx, ly - 1);
		gc->draw(STRINGD->chars('-', sx));
		gc->move_pen(lx, hy + 1);
		gc->draw(STRINGD->chars('-', sx));

		for (i = ly; i <= hy; i++) {
			gc->move_pen(lx - 1, i);
			gc->draw("|");
			gc->move_pen(hx + 1, i);
			gc->draw("|");
		}
	} else {
		int i;

		/* corners */
		gc->set_color(0x07);
		gc->move_pen(lx, ly);
		gc->draw("+");
		gc->move_pen(hx, ly);
		gc->draw("+");
		gc->move_pen(lx, hy);
		gc->draw("+");
		gc->move_pen(hx, hy);
		gc->draw("+");

		/* walls */
		if (sx > 2) {
			gc->move_pen(lx + 1, ly);
			gc->draw(STRINGD->chars('-', sx - 2));
			gc->move_pen(lx + 1, hy);
			gc->draw(STRINGD->chars('-', sx - 2));
		}

		if (sy > 2) {
			for (i = ly + 1; i <= hy - 1; i++) {
				gc->move_pen(lx, i);
				gc->draw("|");
				gc->move_pen(hx, i);
				gc->draw("|");
			}
		}

		gc->set_color(0x08);
		/* roof */
		if (sx > 2 && sy > 2) {
			for (i = ly + 1; i <= hy - 1; i++) {
				gc->move_pen(lx + 1, i);
				gc->draw(STRINGD->spaces(sx - 2));
			}
		}
	}
}
